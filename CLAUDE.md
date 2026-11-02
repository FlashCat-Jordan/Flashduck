# Claude / Coding Agent Guide

This repository contains **two Flashduck firmware systems** plus a development skill:

- voice assistant: `firmware/voice/` (MIT);
- DuckClaw local agent: `firmware/duckclaw/` (CC BY-NC-SA 4.0);
- skill: `skills/flashduck-dev/`.

Read [`AGENTS.md`](AGENTS.md), then [`docs/architecture.md`](docs/architecture.md), then the skill's `SKILL.md`.

## Project facts

- **Voice board project:** `firmware/voice/main/boards/flashduck`.
- **DuckClaw:** independent ESP-IDF project under `firmware/duckclaw`.
- Optional camera: OV2640 / OV3660 / GC2145; boot continues without one.
- MCU: ESP32-S3. Framework: ESP-IDF 6.0.1+. UI: LVGL.
- Voice wake word: `wn9_xiaoyaxiaoya_tts2` (“小鸭小鸭”).
- The wake model and audio live in the **assets partition**; reflashing only the app can leave the old audio active.
- DuckClaw does not read camera images and is not MIT.

## Quick commands

```bash
# Environment check
bash skills/flashduck-dev/scripts/check-env.sh

# Voice
cd firmware/voice
python3 scripts/build.py --list-boards
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
python3 -m unittest discover -s scripts/tests -v

# DuckClaw
cd ../duckclaw
idf.py set-target esp32s3
idf.py build
idf.py merge-bin
```

Voice full image: `firmware/voice/build/merged-binary.bin` at `0x0`; app at `0x20000`; assets at `0x800000`.

## Rules

- Do not add another board unless the user explicitly asks. The voice firmware contains only the Flashduck board.
- Do not copy DuckClaw code into the MIT voice firmware without checking CC BY-NC-SA.
- Do not modify `build/`, `managed_components/`, `sdkconfig*`, `main/assets/lang_config.h`, or generated mmap headers.
- Keep board-specific behavior inside the board directory; core code uses `Board` interfaces.
- Preserve existing pins. If a new hardware revision is needed, add a uniquely named board/variant.
- Mouth 45–60°, neck 45–180°.
- A successful compile is not hardware validation. Call out servo direction, camera detection, audio, battery and Wi-Fi checks.

## Where to look

| Topic | File |
|---|---|
| Agent-readable summary | `llms.txt` |
| Build/test rules | `AGENTS.md` |
| Architecture | `docs/architecture.md` |
| Agent workflow | `docs/coding-agent.md` |
| Skill | `skills/flashduck-dev/SKILL.md` |
| Voice board | `firmware/voice/main/boards/flashduck/README.md` |
| Voice Kconfig | `firmware/voice/main/Kconfig.projbuild` |
| Voice build source selection | `firmware/voice/main/CMakeLists.txt` |
| DuckClaw | `firmware/duckclaw/README.md` |
