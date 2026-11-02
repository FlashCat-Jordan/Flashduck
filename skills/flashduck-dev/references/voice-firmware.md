# Voice Firmware Map

Root: `firmware/voice`

## Build and flash

```bash
cd firmware/voice
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
```

Outputs:

- `build/merged-binary.bin` — full image at `0x0`
- `build/xiaozhi.bin` — app at `0x20000`
- `build/generated_assets.bin` — assets at `0x800000`

Full flash:

```bash
esptool --chip esp32s3 -p <PORT> -b 460800 write_flash 0x0 build/merged-binary.bin
```

If only the app changed, `0x20000` is enough. If the wake word or sounds changed, reflash the assets at `0x800000`; otherwise the old wake model may remain.

## Code map

| Area | Files |
|---|---|
| Board pins/config | `main/boards/flashduck/config.h` |
| Board init / camera | `main/boards/flashduck/flashduck_robot.cc` |
| Eye/display | `main/boards/flashduck/flashduck_display.cc/.h` |
| Actions + MCP registration | `main/boards/flashduck/flashduck_controller.cc/.h` |
| Motion primitives | `main/boards/flashduck/flashduck_movements.cc/.h` |
| Web gait page | `main/boards/flashduck/flashduck_web_server.cc` |
| Common MCP tools/camera tool | `main/mcp_server.cc` |
| Audio/wake word | `main/audio/` |
| Build entry | `scripts/build.py` |
| Board docs | `main/boards/flashduck/README.md`, `docs/` |

## Eye

`FlashduckDisplay` has:

```cpp
enum class EyeState { Idle, Scanning, Connected, Listening, Speaking };
struct EyeStyle {
  float upper_lid, lower_lid, pupil_scale, gaze_x, gaze_y;
  uint32_t move_min_ms, move_max_ms, blink_min_ms, blink_max_ms;
  lv_color_t iris_color;
};
```

`StyleFor(EyeState)` returns the style. To change an existing expression, edit that style table. To add a new state:

1. Add the enum value.
2. Add a style and a `case`.
3. Call `SetEyeState()` when the app enters that state.
4. Build and flash.

Do not add GIF assets for the eye; it is drawn in LVGL in real time.

## Actions

Predefined actions are registered in `flashduck_controller.cc`:

- `ActionType` enum;
- `ActionTask` switch that calls `duck_.Walk()`, `HeadTurn()`, `Quack()`, etc.;
- `self.flashduck.action` MCP tool that maps action name strings to `QueueAction()`.

To add an action:

1. Add a motion method in `flashduck_movements.cc/.h`, or compose existing methods.
2. Add `ACTION_*` to `ActionType`.
3. Add a `case` in `ActionTask`.
4. Add an `if/else` branch to the MCP tool and update its description.
5. Build, flash, and test the servo limits.

## Gait

`FlashduckGaitParams` contains hip offset/amplitude, foot amplitude, phase, speed and head swing.

- Web page: `http://<device-ip>/`
- NVS namespace: `flashduck_gait`
- Defaults: `flashduck_movements.h`
- Web server: `flashduck_web_server.cc`

When changing defaults, remember NVS values override code defaults on an existing device.

## Camera / vision

`main/mcp_server.cc` registers `self.camera.take_photo` only when `Board::GetCamera()` is non-null. The tool captures a frame and calls `Camera::Explain(question)` through the configured backend. If no camera is installed, the firmware skips initialization and continues booting.

## Tests

```bash
cd firmware/voice
python3 -m unittest discover -s scripts/tests -v
```

A build and a test pass are not hardware validation.
