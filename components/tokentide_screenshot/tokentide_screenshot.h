#pragma once

#include "esphome/core/component.h"
#include "esphome/components/lvgl/lvgl_proxy.h"

#include <esp_http_server.h>

namespace esphome {
namespace tokentide_screenshot {

// Dev/QA helper: GET /screenshot on a tiny embedded HTTP server returns
// the current LVGL screen as a 24-bit BMP. The snapshot itself runs in
// loop() (LVGL is not thread-safe); the server task hands the request
// over via a small state machine and streams the finished buffer.
// LAN-only and unauthenticated - opt-in, not part of the default config.
class TokentideScreenshot : public Component {
 public:
  void set_port(uint16_t port) { this->port_ = port; }
  void setup() override;
  void loop() override;
  float get_setup_priority() const override { return setup_priority::AFTER_WIFI; }

  enum ShotState : uint8_t { IDLE, REQUESTED, READY, STREAMED };

  volatile ShotState state_{IDLE};
  lv_draw_buf_t *buf_{nullptr};

 protected:
  static esp_err_t handler_(httpd_req_t *req);

  uint16_t port_{8081};
  httpd_handle_t server_{nullptr};
};

}  // namespace tokentide_screenshot
}  // namespace esphome
