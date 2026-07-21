# clawd-bot

<p align="center">
  <img src="assets/logo.png" alt="clawd-bot logo" width="256"/>
</p>

A standalone Claude Code usage monitor for the desk: big screen, readable
from across the room, no host software. It runs on the M5Stack CoreS3 (the
head unit of the [Stack-chan](https://docs.m5stack.com/en/StackChan/) robot
kit), connects to your WiFi, and polls Anthropic directly with a dedicated
long-lived token.

Built as an ESPHome package: adopt it from the ESPHome Builder (inside or
outside Home Assistant) with a dozen lines of YAML, update over-the-air by
bumping a release tag.

## What it shows

- **5-hour window utilization** as a hero number readable from desk distance
- 5h and 7-day bars with reset countdowns
- Status line: limit status, battery, and the polling cost (requests/day)
  so you always know what the monitor itself consumes
- On-screen touch button to force a refresh

## Supported devices

One codebase, per-board implementations: every board gets its own entry
under `boards/` combining the shared usage engine with hardware support
and a UI sized to what the device can do. All boards update the same way
(bump the release tag, flash OTA).

| Board | Import file | Status | UI |
|---|---|---|---|
| [M5Stack CoreS3](https://docs.m5stack.com/en/core/CoreS3) / [Stack-chan](https://docs.m5stack.com/en/StackChan/) | `boards/m5stack-cores3.yaml` | supported | rich 320x240 touch: hero number, bars, countdowns, touch refresh |
| M5Stack CoreS3 SE | `boards/m5stack-cores3.yaml` | untested, should work | same as CoreS3 (no battery gauge) |
| [M5StickC Plus](https://docs.m5stack.com/en/core/m5stickc_plus) | `boards/m5stickc-plus.yaml` | supported, tested on hardware | minimal 240x135: hero number, 7d bar, countdown; front button = refresh |
| M5StickC Plus + [Joystick Hat](https://docs.m5stack.com/en/hat/hat-joystick) | `boards/m5stickc-plus-joy.yaml` | supported | minimal + joystick: press = refresh, Y-axis = brightness |
| M5StickC Plus2 | planned | - | minimal (same UI class) |

Want another board? See [CONTRIBUTING.md](CONTRIBUTING.md) - the layout
is designed for drive-in board additions.

## How it gets the data

Anthropic exposes the unified rate-limit state as response headers on API
calls. clawd-bot sends a minimal ~1-token probe request to `/v1/messages`
with your Claude Code OAuth token and reads
`anthropic-ratelimit-unified-{5h,7d}-{utilization,reset}` from the response.
At the default 120s interval that is ~720 tiny requests/day; the footer
shows the figure for your configured interval.

The token comes from `claude setup-token` (valid one year, scoped to
inference). Treat it like a password: it lives in your ESPHome secrets and
never leaves the device except toward api.anthropic.com over TLS.

## Quick start - ESPHome Builder (Home Assistant or standalone)

1. Generate a token on any machine with Claude Code:

   ```
   claude setup-token
   ```

2. In the ESPHome Builder, add these secrets (Secrets editor):

   ```yaml
   wifi_ssid: "YourWiFi"
   wifi_password: "YourPassword"
   ap_password: "clawd-bot"
   claude_token: "sk-ant-oat01-..."
   ```

3. Create a new device with this config:

   ```yaml
   substitutions:
     name: clawd-bot
     claude_token: !secret claude_token
     # poll_interval: "120"   # optional override, seconds

   packages:
     clawd_bot:
       url: https://github.com/eldios/clawd-bot
       ref: v0.0.1
       files: [boards/m5stack-cores3.yaml]
       refresh: 1d

   wifi:
     ssid: !secret wifi_ssid
     password: !secret wifi_password
     ap:
       password: !secret ap_password
   ```

4. First install: connect the CoreS3 over USB and use "Install via USB"
   (or [web.esphome.io](https://web.esphome.io) from any browser).
5. Updates: bump `ref:` to the new release tag and hit Install - it goes
   over-the-air, no cable needed.

## Quick start - CLI

```
git clone https://github.com/eldios/clawd-bot
cd clawd-bot
cp secrets.example.yaml secrets.yaml   # fill wifi + claude_token
esphome run clawd-bot.yaml             # first time over USB, then OTA
```

Nix users: `nix develop` provides `esphome` and `esptool`.

## Languages

UI strings default to English. Ready-made packs live in `lang/`
(`it`, `de`, `fr`, `es`): include one after the board package, or override
individual `str_*` substitutions directly in your config (top-level
substitutions always win).

```yaml
packages:
  clawd_bot:
    url: https://github.com/eldios/clawd-bot
    ref: v0.0.1
    files: [boards/m5stack-cores3.yaml, lang/it.yaml]
    refresh: 1d
```

## Home Assistant integration

The device exposes its sensors natively: 5h/7d utilization, limit status,
battery. Automations like "notify me at 80% usage" are a two-line HA
automation away - no extra firmware work.

## Roadmap

- Session-reset chime (the CoreS3 speaker + AW88298 amp are supported by
  M5Stack's official ESPHome components already)
- Anthropic status-page indicator
- Stack-chan body features: servo gestures, LEDs, and friends

## Credits

- Data mechanism inspired by
  [claude-usage-stick](https://github.com/oauramos/claude-usage-stick)
- Desk-distance UX inspired by
  [Clawdmeter](https://github.com/HermannBjorgvin/Clawdmeter)
- CoreS3 hardware support via
  [M5Stack's official ESPHome components](https://github.com/m5stack/esphome-yaml)

## License

MIT - see [LICENSE](LICENSE).
