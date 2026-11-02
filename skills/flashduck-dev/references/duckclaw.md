# DuckClaw Map

Root: `firmware/duckclaw`

DuckClaw is an **on-device agent** firmware, adapted from OttoClaw for Flashduck. Its agent loop, memory, tool dispatch and body control run inside the robot. It is a separate ESP-IDF project and is **not** the voice firmware.

## Build and flash

```bash
cd firmware/duckclaw
idf.py set-target esp32s3
idf.py build
idf.py merge-bin
esptool --chip esp32s3 -p <PORT> -b 460800 write_flash 0x0 build/merged-binary.bin
```

The app partition is tight (about 3% free in the current build). If you add much code, enlarge the app partitions in `partitions.csv` before adding more features.

## Code map

| Area | Files |
|---|---|
| Boot / services | `main/duckclaw.c` |
| Board pins | `main/board/board_config.h` |
| Kconfig | `main/Kconfig.projbuild` |
| Motion primitives | `main/otto/otto_movements.c/.h` |
| Action + pose tools | `main/tools/tool_otto.c` |
| Agent loop | `main/agent/agent_loop.c` |
| DingTalk | `main/dingtalk/dingtalk_bot.c` |
| LCD | `main/lcd/lcd_display.c` |
| Config portal | `main/config_portal/config_portal.c` |
| Prompts, tools, memory | `spiffs_data/config/`, `spiffs_data/memory/` |

## Hardware adaptation

- Uses the Flashduck camera-board routing.
- Left/right hand outputs are the **mouth** (GPIO4) and **neck** (GPIO7).
- `has_hands = false` disables the old humanoid arm actions.
- Duck actions are in `tool_otto.c`: `head_turn`, `head_shake`, `look_around`, `quack`, `chatter`, `happy_waddle`, `preen`, `peck`.
- Pose fields are `mouth` (45–60) and `neck` (45–180); old `left_hand`/`right_hand` names are accepted as aliases.

## Add a DuckClaw action

1. Add a helper/sequence in `tool_otto.c` using `otto_move_servos()`.
2. Add an `else if` branch in `tool_otto_action_execute()`.
3. Add the name to the action schema and `spiffs_data/config/TOOLS.md`.
4. Update `spiffs_data/config/AGENTS.md` if the agent should prefer it.
5. Build and flash.

Never exceed mouth 45–60° or neck 45–180°.

## Configuration

- Captive portal: first boot or hold BOOT; hotspot `DuckClaw-XXXX`.
- Build-time secrets: copy `main/duckclaw_secrets.h.example` to `main/duckclaw_secrets.h`; it is gitignored.
- DingTalk / model / proxy fields can also be set in the config page.

## Limitations

- The current DuckClaw on-device agent does **not** read camera images.
- The LCD UI still uses the original pixel-robot interface, not a duck-specific redraw.
- Local agent behavior is driven by the model and prompts; a compile pass does not validate the conversation.

## License

`firmware/duckclaw/` is **CC BY-NC-SA 4.0**, not MIT. Do not copy it into the MIT voice firmware without checking the license.
