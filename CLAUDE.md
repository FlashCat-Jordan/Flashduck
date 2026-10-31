# Claude / Coding Agent Guide

This repository contains **two Flashduck firmware systems**: the voice assistant
in the root and **DuckClaw** in `duckclaw/`. Read
[`AGENTS.md`](AGENTS.md) first for the full project rules, then use this file
for the shortest safe path.

## Project facts

- **Voice assistant board project:** `main/boards/flashduck`.
- **DuckClaw:** independent ESP-IDF project under `duckclaw/` (CC BY-NC-SA 4.0).
- Optional camera: OV2640 / OV3660 / GC2145; boot continues without one.
- MCU: ESP32-S3. Framework: ESP-IDF 6.0.1+. UI: LVGL.
- Wake word in the Flashduck build command: `wn9_xiaoyaxiaoya_tts2`
  (spoken as “小鸭小鸭”).
- The wake model and audio live in the **assets partition**. Reflashing only the
  app can leave the old wake word/audio active.

## Quick commands

```bash
python3 scripts/build.py --list-boards
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
python3 -m unittest discover -s scripts/tests -v

# DuckClaw (separate project, separate partition table)
cd duckclaw && idf.py set-target esp32s3 && idf.py build
```

Full image is `build/merged-binary.bin`, flashed at `0x0`. App only is the
app image at `0x20000`; assets are `build/generated_assets.bin` at `0x800000`.

## Rules

- Do not add another board unless the user explicitly asks. The voice firmware
  contains only the Flashduck board; DuckClaw is a separate system.
- `duckclaw/` is licensed CC BY-NC-SA 4.0, not MIT. Do not copy its code into
  the MIT root without checking the license.
- Do not modify `build/`, `managed_components/`, `sdkconfig*`,
  `main/assets/lang_config.h`, or generated mmap headers.
- Keep board-specific behavior inside the board directory; core code uses the
  `Board` interfaces.
- Preserve existing pins. If a new hardware revision is needed, add a uniquely
  named board/variant instead of changing an existing one.
- A successful compile is not hardware validation. Call out servo direction,
  camera detection, audio, battery and Wi-Fi checks that still need a real board.

## Where to look

| Topic | File |
|---|---|
| Agent-readable summary | `llms.txt` |
| Build/test rules | `AGENTS.md` |
| Contribution guide | `CONTRIBUTING.md` |
| Flashduck firmware | `main/boards/flashduck/README.md` |
| Kconfig board selection | `main/Kconfig.projbuild` |
| Board source selection | `main/CMakeLists.txt` |
