#include "clawd_screenshot.h"
#include "esphome/core/log.h"

#include <cstring>

namespace esphome {
namespace clawd_screenshot {

static const char *const TAG = "clawd_screenshot";

void ClawdScreenshot::setup() {
  httpd_config_t cfg = HTTPD_DEFAULT_CONFIG();
  cfg.server_port = this->port_;
  cfg.ctrl_port = this->port_ + 10000;
  if (httpd_start(&this->server_, &cfg) != ESP_OK) {
    ESP_LOGW(TAG, "Failed to start screenshot server on port %u", this->port_);
    this->mark_failed();
    return;
  }
  httpd_uri_t uri{};
  uri.uri = "/screenshot";
  uri.method = HTTP_GET;
  uri.handler = ClawdScreenshot::handler_;
  uri.user_ctx = this;
  httpd_register_uri_handler(this->server_, &uri);
  ESP_LOGI(TAG, "Screenshot server listening on port %u, path /screenshot", this->port_);
}

void ClawdScreenshot::loop() {
  if (this->state_ == REQUESTED) {
    this->buf_ = lv_snapshot_take(lv_scr_act(), LV_COLOR_FORMAT_RGB565);
    this->state_ = READY;
  } else if (this->state_ == STREAMED) {
    if (this->buf_ != nullptr) {
      lv_draw_buf_destroy(this->buf_);
      this->buf_ = nullptr;
    }
    this->state_ = IDLE;
  }
}

esp_err_t ClawdScreenshot::handler_(httpd_req_t *req) {
  auto *self = static_cast<ClawdScreenshot *>(req->user_ctx);
  if (self->state_ != IDLE) {
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "screenshot busy");
    return ESP_OK;
  }
  self->state_ = REQUESTED;
  for (int i = 0; i < 60 && self->state_ != READY; i++)
    vTaskDelay(pdMS_TO_TICKS(50));
  if (self->state_ != READY || self->buf_ == nullptr) {
    self->state_ = STREAMED;
    httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "snapshot failed (out of memory?)");
    return ESP_OK;
  }

  const lv_draw_buf_t *buf = self->buf_;
  int w = buf->header.w, h = buf->header.h;
  uint32_t stride = buf->header.stride;
  uint32_t row_bytes = ((uint32_t) w * 3 + 3) & ~3u;
  uint32_t data_size = row_bytes * h;

  // 54-byte BMP header, 24bpp BGR, negative height = top-down rows.
  uint8_t hdr[54] = {};
  uint32_t file_size = 54 + data_size;
  hdr[0] = 'B'; hdr[1] = 'M';
  memcpy(hdr + 2, &file_size, 4);
  uint32_t off = 54, hsz = 40;
  memcpy(hdr + 10, &off, 4);
  memcpy(hdr + 14, &hsz, 4);
  int32_t bw = w, bh = -h;
  memcpy(hdr + 18, &bw, 4);
  memcpy(hdr + 22, &bh, 4);
  uint16_t planes = 1, bpp = 24;
  memcpy(hdr + 26, &planes, 2);
  memcpy(hdr + 28, &bpp, 2);
  memcpy(hdr + 34, &data_size, 4);

  httpd_resp_set_type(req, "image/bmp");
  if (httpd_resp_send_chunk(req, (const char *) hdr, sizeof(hdr)) != ESP_OK) {
    self->state_ = STREAMED;
    return ESP_FAIL;
  }
  auto *row = (uint8_t *) malloc(row_bytes);
  if (row == nullptr) {
    self->state_ = STREAMED;
    httpd_resp_send_chunk(req, nullptr, 0);
    return ESP_FAIL;
  }
  memset(row, 0, row_bytes);
  esp_err_t err = ESP_OK;
  for (int y = 0; y < h && err == ESP_OK; y++) {
    const uint16_t *src = (const uint16_t *) (buf->data + (size_t) y * stride);
    for (int x = 0; x < w; x++) {
      uint16_t v = src[x];
      row[x * 3 + 0] = (uint8_t)((v & 0x1F) << 3);           // B
      row[x * 3 + 1] = (uint8_t)(((v >> 5) & 0x3F) << 2);    // G
      row[x * 3 + 2] = (uint8_t)(((v >> 11) & 0x1F) << 3);   // R
    }
    err = httpd_resp_send_chunk(req, (const char *) row, row_bytes);
  }
  free(row);
  httpd_resp_send_chunk(req, nullptr, 0);
  self->state_ = STREAMED;
  return err;
}

}  // namespace clawd_screenshot
}  // namespace esphome
