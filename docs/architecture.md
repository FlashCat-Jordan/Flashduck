# Architecture

## 1. Product logic

Flashduck is one six-servo desktop robot with **two independent interaction systems**. The user chooses one, not both:

```text
                    Flashduck hardware
                  ESP32-S3 + 6 servos
                  display / camera / audio
                           |
          +----------------+----------------+
          |                                 |
   firmware/voice                    firmware/duckclaw
   voice assistant                   DuckClaw on-device agent
   wake word + conversation          DingTalk + local memory
   eye + actions + gait              emotions + relationship
   MCP tools + camera                AI-controlled poses
          |                                 |
          +-------- one system at a time -----+
```

They share the same board, shell and servos, but they are separate ESP-IDF projects with separate partition tables and flash images. Only one system runs at a time; flashing one replaces the other.

The third layer is not firmware:

```text
skills/flashduck-dev/
  SKILL.md        decision tree + safety + workflow
  references/     hardware, voice, DuckClaw, recipes, environment
  scripts/        check/install env, build, flash, ASCII-copy helper
```

The skill tells a human or coding agent **how to work on the two firmware projects**; it does not run on the robot.

## 2. Repository layers

| Layer | Path | Responsibility |
|---|---|---|
| Product | `README.md`, `README_EN.md` | What it is, play styles, four-step build |
| Firmware | `firmware/voice/`, `firmware/duckclaw/` | Buildable code |
| Skill | `skills/flashduck-dev/` | Agent/beginner operating manual |
| Cross-project docs | `docs/` | Architecture and agent workflow |
| Voice docs | `firmware/voice/docs/` | Voice-specific technical docs |
| Shared media | `firmware/voice/docs/images/` | README images and QR codes |

**Rule:** product-level text belongs in root README; voice-only technical detail belongs under `firmware/voice/docs`; DuckClaw behavior belongs under `firmware/duckclaw`; agent instructions belong in `skills/` and `docs/coding-agent.md`.

## 3. Voice firmware — code logic

Root: `firmware/voice`

```text
main/
  application.*            event loop, state machine, protocol lifecycle
  device_state_machine.*   legal runtime states
  boards/
    flashduck/             board pins, camera, display, actions, web
    common/                board interfaces, audio/network/display helpers
  audio/                   codecs, wake word, AFE, audio tasks
  protocols/               WebSocket / MQTT / UDP transport
  display/                 reusable LVGL display and themes
  mcp_server.*             device-side MCP tools
  assets/                  sounds, languages, wake model
scripts/build.py           board/variant build entry
partitions/                app/assets partition tables
```

### Boot and interaction flow

```text
boot -> board init -> display/eye + audio + Wi-Fi
     -> provisioning / backend connection
     -> wake word -> audio stream -> backend
     -> response -> speaker + eye state + optional MCP action
```

### Code ownership

| Change | Owner |
|---|---|
| GPIO, display, camera wiring | `main/boards/flashduck/config.h` |
| Board initialization / camera detection | `main/boards/flashduck/flashduck_robot.cc` |
| Eye state/style | `main/boards/flashduck/flashduck_display.cc/.h` |
| Predefined action registry | `main/boards/flashduck/flashduck_controller.cc/.h` |
| Motion primitives | `main/boards/flashduck/flashduck_movements.cc/.h` |
| Web gait page | `main/boards/flashduck/flashduck_web_server.cc` |
| MCP tools / camera tool | `main/mcp_server.cc` and board controller |
| Wake word/audio | `main/audio/` and build flags |

### Eye

The eye is drawn in LVGL. `FlashduckDisplay` owns `EyeState` and `EyeStyle`; `StyleFor()` maps each state to lids, pupil scale, gaze and iris color. There is no GIF or image asset for the eye. Add a state by extending the enum, style table and the call sites that change state.

### Actions

`flashduck_controller.cc` has three layers:

1. `ActionType` enum — internal action IDs.
2. `ActionTask` switch — calls `Flashduck` motion methods.
3. `self.flashduck.action` MCP tool — maps model-facing action names to the queue.

To add an action, change all three; otherwise the model cannot call it or the controller does not execute it.

### Gait

`FlashduckGaitParams` holds hip offset/amplitude, foot amplitude, phase, speed and head swing. The web page writes them to NVS (`flashduck_gait`). NVS values override code defaults on an existing device; mention this when a user says “I changed the default but nothing happened”.

### Camera

The camera is optional. `flashduck_robot.cc` detects OV2640/OV3660/GC2145; `mcp_server.cc` registers `self.camera.take_photo` only when `GetCamera()` is non-null. The photo explanation is a backend capability, not a local vision model.

## 4. DuckClaw — on-device agent code logic

Root: `firmware/duckclaw`

```text
main/
  duckclaw.c               boot, services, dispatcher
  board/board_config.h     Flashduck camera-board pins
  agent/                   agent loop and context
  dingtalk/                DingTalk bot transport
  tools/tool_otto.c        action + pose tools
  otto/otto_movements.c    six-servo motion primitives
  lcd/                     pixel UI
  config_portal/           captive portal
spiffs_data/
  config/                  IDENTITY, SOUL, AGENTS, TOOLS, RELATION
  memory/                  local memory
  www/                     setup/settings pages
```

DuckClaw is an **on-device agent**: its loop, sessions, memory, tool dispatch and
body control run inside the robot. It can call external tools, connect to social
software such as DingTalk, and drive the six servos directly.

### Interaction flow

```text
DingTalk message -> dingtalk_bot -> on-device agent loop
  -> model/tool call -> external tool or tool_otto (action/pose)
  -> servos + LCD emotion
  -> reply -> DingTalk
```

### Hardware adaptation

DuckClaw reuses the Flashduck camera-board routing:

- mouth on GPIO4 (former left-hand output);
- neck on GPIO7 (former right-hand output);
- legs/feet on GPIO5/43/6/44;
- old humanoid arm actions are disabled (`has_hands = false`);
- duck actions live in `tool_otto.c`;
- mouth is clamped to 45–60°, neck to 45–180°.

### Limitations

- The current DuckClaw agent does **not** read camera images; the camera-board version only uses that board's pin layout.
- The LCD still uses the original pixel-robot UI, not a duck-specific redraw.
- App partition space is tight; enlarge the partition table before a large feature addition.

## 5. Build and flash boundary

| Firmware | Project root | Full image | App | Assets / SPIFFS |
|---|---|---|---|---|
| Voice | `firmware/voice` | `build/merged-binary.bin` @ `0x0` | `0x20000` | assets @ `0x800000` |
| DuckClaw | `firmware/duckclaw` | `build/merged-binary.bin` @ `0x0` | `0x20000` | SPIFFS @ `0x420000` |

Do not flash a voice image onto DuckClaw or the reverse; they have different partition tables and app layouts.

## 6. License boundary

- Root voice firmware: **MIT**.
- `firmware/duckclaw/`: **CC BY-NC-SA 4.0**.
- Do not copy DuckClaw code into the MIT voice project without checking the license.
- The skill and docs are repository documentation; they do not change either firmware license.

## 7. What to read next

- New user: root README.
- Coding agent: `docs/coding-agent.md` then `skills/flashduck-dev/SKILL.md`.
- Voice change: `skills/flashduck-dev/references/voice-firmware.md`.
- DuckClaw change: `skills/flashduck-dev/references/duckclaw.md`.
- Hardware: `skills/flashduck-dev/references/hardware.md`.
