# Recipes

All paths are inside this repository. Run commands from the stated directory.

## 1. Change an eye color

File: `firmware/voice/main/boards/flashduck/flashduck_display.cc`

Find `StyleFor()` and change the `iris_color` of the state, for example:

```cpp
case EyeState::Listening:
    return {0.00f, 0.00f, 1.35f, 3.0f, 2.0f, 1500, 3000, 5000, 9000,
            lv_color_hex(0x9B7BFF)};
```

Build:

```bash
cd firmware/voice
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
```

Flash `build/merged-binary.bin` at `0x0`.

## 2. Add a new voice action

Use the existing action pipeline, not a new task.

Files:

- `firmware/voice/main/boards/flashduck/flashduck_movements.cc/.h`
- `firmware/voice/main/boards/flashduck/flashduck_controller.cc`

Steps:

1. Add a method (or compose existing motion methods) in `flashduck_movements`.
2. Add `ACTION_*` to `ActionType`.
3. Add the `case` in `ActionTask`.
4. Add the action name in the `self.flashduck.action` MCP tool and its description.
5. Build and flash.
6. Test the physical range before closing the shell.

Do not send mouth values above 60° or neck values outside 45–180°.

## 3. Change gait defaults

File: `firmware/voice/main/boards/flashduck/flashduck_movements.h`

Look for `FLASHDUCK_WALK_*` defines. If the device already has saved NVS values, changing code defaults will not override them; use the web page and **save as default**, or erase the `flashduck_gait` NVS namespace.

## 4. Add a voice MCP tool

File: `firmware/voice/main/mcp_server.cc`

Use `AddTool(name, description, properties, callback)`. If the tool is board-specific, put it in `flashduck_controller.cc` instead and register it from the board initialization.

Test the tool from the voice assistant or MCP client. Do not expose raw GPIO control to the model.

## 5. Use the camera

The voice firmware registers `self.camera.take_photo` when a camera is detected.

- Ask: “看看我手里是什么？”
- Requires a backend that supports image explanation.
- If the camera is absent, the firmware skips it and still boots.
- DuckClaw does not currently use camera images.

## 6. Add a DuckClaw action

File: `firmware/duckclaw/main/tools/tool_otto.c`

1. Add a helper such as `duck_preen()` using `otto_move_servos()`.
2. Add an `else if (strcmp(action_name, "your_action") == 0)` branch.
3. Add the name to the JSON schema.
4. Add it to `firmware/duckclaw/spiffs_data/config/TOOLS.md`.
5. Optionally mention it in `AGENTS.md`.
6. Build with `idf.py build` and flash.

Use only safe angles: mouth 45–60, neck 45–180.

## 7. Change the DuckClaw prompt

Files:

- `firmware/duckclaw/spiffs_data/config/IDENTITY.md`
- `SOUL.md`
- `AGENTS.md`
- `TOOLS.md`

After editing, rebuild; SPIFFS is generated into the image. If you only flash the app, the old prompts remain.

## 8. Tune a servo trim

Voice firmware: `firmware/voice/main/boards/flashduck/config.h` has trim fields; the web page can also expose/save gait, but trim is board configuration.

DuckClaw: `main/otto/otto_movements.c` has trim support; do not guess values. Move one joint at a time, power off between changes.

## 9. Build and flash a full image

```bash
cd firmware/voice
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
esptool --chip esp32s3 -p <PORT> -b 460800 write_flash 0x0 build/merged-binary.bin
```

DuckClaw:

```bash
cd firmware/duckclaw
idf.py set-target esp32s3
idf.py build
idf.py merge-bin
esptool --chip esp32s3 -p <PORT> -b 460800 write_flash 0x0 build/merged-binary.bin
```

## 10. Beginner first task

Ask the user to choose:

- “让眼睛换个颜色” → recipe 1;
- “加一个鸭子动作” → recipe 2;
- “改 DuckClaw 的性格/说话方式” → recipe 7.

Then check the environment, make one change, build, flash, and ask for a photo/video or serial log. Do not change pins on the first task.
