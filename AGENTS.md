# AGENTS.md

## Project

This repository contains **two firmware systems** for the Flashduck robot plus a coding-agent skill:

- `firmware/voice/` — voice assistant (one board project: Flashduck);
- `firmware/duckclaw/` — DuckClaw local agent (separate ESP-IDF project, CC BY-NC-SA 4.0);
- `skills/flashduck-dev/` — environment checks, recipes and agent workflow.

They cannot run at the same time. The shared hardware is described in the skill and root README.

Use the ESP-IDF `release/v6.1` branch when possible. The minimum supported SDK is ESP-IDF v6.0.1. IDF 5.x is not supported.

## First read

- `skills/flashduck-dev/SKILL.md` — environment, safety, recipes;
- `docs/architecture.md` — project/code logic;
- `docs/coding-agent.md` — agent workflow;
- `firmware/README.md` — build boundaries.

## Architecture

### Voice firmware (`firmware/voice/`)

- `main/application.*`: event loop, protocol lifecycle, high-level behavior.
- `main/device_state_machine.*`: legal runtime state transitions.
- `main/boards/common/`: board interfaces and reusable hardware/network helpers.
- `main/boards/flashduck/`: board pins, camera, display, actions, web.
- `main/audio/`: codecs, audio tasks, wake words, queues.
- `main/protocols/`: WebSocket / MQTT / UDP.
- `main/display/` and `main/led/`: reusable UI.
- `main/mcp_server.*`: common device-side MCP tools and camera tool.
- `main/Kconfig.projbuild`: board/feature configuration.
- `main/CMakeLists.txt`: source/board/locale/font/asset selection.
- `scripts/build.py`: canonical board/variant build entry.
- `docs/`: voice-specific documentation and images.

### DuckClaw (`firmware/duckclaw/`)

- `main/duckclaw.c`: boot and services;
- `main/board/board_config.h`: Flashduck camera-board pins;
- `main/agent/`, `main/dingtalk/`: local agent and social channel;
- `main/tools/tool_otto.c`: action + pose tools;
- `main/otto/otto_movements.c`: motion primitives;
- `main/lcd/`, `main/config_portal/`: UI and provisioning;
- `spiffs_data/`: prompts, config pages, memory.

## Required rules

- Preserve unrelated worktree changes and keep patches focused.
- A voice build must export exactly one board factory through `DECLARE_BOARD(...)`.
- Never alter an existing board's pins to support different hardware. Add a uniquely named board or release variant; board identity affects OTA compatibility.
- Core voice code depends on `Board` interfaces, never a concrete board class or board `config.h`.
- Treat camera, backlight, display, LED, battery and similar capabilities as optional.
- Change runtime state through `Application::SetDeviceState()` and the state machine.
- Callbacks may run outside the main task. Schedule application mutations with `Application::Schedule()` or event bits.
- Do not block the main event loop or audio tasks.
- Keep shared message semantics in `Protocol`; verify both transports when changing its contract.
- Validate network input and preserve `cJSON` ownership. NVS keys are persistent API and require migration when changed.
- Guard target-specific features with Kconfig/component rules.
- Do not manually edit generated/vendor output: `build/`, `releases/`, `managed_components/`, `components/`, `sdkconfig*`, `main/assets/lang_config.h`, generated mmap headers.
- Format only touched C/C++ files with the repository `.clang-format`.
- Do not mix the MIT voice firmware and CC BY-NC-SA DuckClaw code.
- Mouth is limited to 45–60°; neck to 45–180°.

## Boards and configuration

Voice board selection chain:

`firmware/voice/main/boards/<board>/config.json` → `firmware/voice/scripts/build.py` → `firmware/voice/main/Kconfig.projbuild` → `firmware/voice/main/CMakeLists.txt` → board source and `config.h`.

When adding a board or variant, update every relevant link in that chain. Include a unique board identity, correct chip target, flash/partition settings, exactly one `DECLARE_BOARD`, and board documentation. Follow `firmware/voice/docs/custom-board.md`.

## Commands

```sh
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

# Environment / agent skill
bash ../../skills/flashduck-dev/scripts/check-env.sh
```

The build script changes local `sdkconfig` and build state. Do not assume the build directory still represents a previous target.

On macOS, build from an ASCII path; non-ASCII paths can fail with `picolibc.specs`.

## Validation

- Board-only change: build affected variants and smoke-test changed hardware.
- Core, common-board, audio, protocol, display, dependency, Kconfig or CMake change: run host tests and build representative affected paths.
- Protocol changes: verify WebSocket and MQTT/UDP when shared behavior changes.
- Audio changes: verify capture, playback, wake/VAD, interruption, reconnect, and applicable AEC modes.
- UI/assets changes: verify applicable display paths and partition size.
- DuckClaw changes: build and merge; check app partition headroom.
- Always report what was tested and what still needs physical hardware. A successful build is not hardware validation.

## Authoritative documentation

- Product overview: `README.md`
- Architecture: `docs/architecture.md`
- Agent workflow: `docs/coding-agent.md`
- Voice board guide: `firmware/voice/docs/custom-board.md`
- Voice audio design: `firmware/voice/main/audio/README.md`
- Voice code style: `firmware/voice/docs/code_style.md`
- Protocols: `firmware/voice/docs/websocket.md`, `mqtt-udp.md`, `mcp-protocol.md`
- DuckClaw: `firmware/duckclaw/README.md`
- Skill: `skills/flashduck-dev/SKILL.md`
