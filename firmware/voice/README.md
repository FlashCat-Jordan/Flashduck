# Flashduck Voice Firmware

This is the voice-assistant firmware for Flashduck. It is the ESP-IDF project in this directory.

## Build

```bash
cd firmware/voice
source ~/esp/esp-idf-v6.0.1/export.sh
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
```

Full image: `build/merged-binary.bin`, flash at `0x0`.

## What is here

| Area | Path |
|---|---|
| Board pins/config | `main/boards/flashduck/config.h` |
| Board init/camera | `main/boards/flashduck/flashduck_robot.cc` |
| Eye/display | `main/boards/flashduck/flashduck_display.cc/.h` |
| Actions + MCP | `main/boards/flashduck/flashduck_controller.cc/.h` |
| Motion primitives | `main/boards/flashduck/flashduck_movements.cc/.h` |
| Web gait page | `main/boards/flashduck/flashduck_web_server.cc` |
| Common MCP tools | `main/mcp_server.cc` |
| Build entry | `scripts/build.py` |
| Technical docs | `docs/` |

## Extend it

- Eye: `flashduck_display.cc` → `EyeState` / `EyeStyle` / `StyleFor()`.
- Action: `flashduck_controller.cc` → `ActionType`, `ActionTask`, MCP name mapping.
- Gait: `flashduck_movements.h` defaults, web page, NVS `flashduck_gait`.
- MCP: `main/mcp_server.cc` or board controller.
- Camera: optional; tool registered only when a camera is detected.

Safety: mouth **45–60°**, neck **45–180°**. See [`../../skills/flashduck-dev/references/hardware.md`](../../skills/flashduck-dev/references/hardware.md).

## License

MIT; see the repository root `LICENSE`.
