# Contributing to clawd-bot

Thanks for helping! A few ground rules keep this repo healthy.

## Dev setup

- Nix: `nix develop` gives you `esphome` and `esptool`.
- Otherwise: any ESPHome >= 2026.4.0 install works (`pipx install esphome`).
- `cp secrets.example.yaml secrets.yaml`, fill in WiFi and your
  `claude setup-token` output, then `esphome run clawd-bot.yaml`.

## Before opening a PR

1. `esphome config clawd-bot.yaml` must pass (schema validation).
2. `esphome compile clawd-bot.yaml` must pass.
3. If you touched UI or hardware bits, test on a real CoreS3 and say so in
   the PR ("tested on hardware: yes/no" plus what you exercised).

## Hardware facts need sources

Pin numbers, register values, and component options must come from a
canonical source - M5Stack's official ESPHome configs
(github.com/m5stack/esphome-yaml), the CoreS3 schematic, or the ESPHome
docs for the exact release. Link the source in the PR description.
"It worked for me" without a source is how ports rot.

## Style

- YAML: two-space indent, comments explain WHY, keep packages focused
  (hardware / poller / ui stay separate).
- Lambdas: keep them short; if C++ grows past a screenful it probably
  belongs in an external component.
- User-facing strings and docs are in English.

## Adding a board

v0.0.1 targets the M5Stack CoreS3 only. If you want another board, open an
issue first: the split to a per-board package layout should happen once,
deliberately, not ad hoc in a drive-by PR.
