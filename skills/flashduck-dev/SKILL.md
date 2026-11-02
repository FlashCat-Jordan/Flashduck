---
name: flashduck-dev
description: Use when helping someone build, flash, modify, or extend a Flashduck robot — the voice firmware in firmware/voice, the DuckClaw on-device agent in firmware/duckclaw, or the shared six-servo hardware. Covers environment checks/install, compile/flash, eyes, actions, gait, web tuning, MCP tools, DuckClaw tools/prompts, and beginner-friendly secondary development.
---

# Flashduck Development Skill

Use this skill when the user wants to **build, flash, modify, debug, or teach someone about Flashduck**.

The repository has three layers. Do not mix them:

| Layer | Path | What it owns |
|---|---|---|
| Product | `README.md` | What Flashduck is, how to play, how to choose a system |
| Firmware | `firmware/voice/`, `firmware/duckclaw/` | Two independent ESP-IDF projects |
| Secondary development | `skills/flashduck-dev/` | This skill: environment, recipes, checks |

Read `docs/architecture.md` for the full project/code map and `docs/coding-agent.md` for the agent workflow.

---

## 0. Always check the environment first

Run:

```bash
bash skills/flashduck-dev/scripts/check-env.sh
```

It checks:

- macOS/Linux, Python 3, Git, CMake, Ninja;
- ESP-IDF presence and version (need **6.0.1+**);
- `esptool`;
- whether the repository path is ASCII (required by this project on macOS);
- serial ports, if present.

Then:

- If everything is **OK**, continue.
- If something is **MISSING**, read `references/environment.md`, explain what is missing, and ask before installing. After approval run:

```bash
bash skills/flashduck-dev/scripts/setup-env.sh --yes
```

- If ESP-IDF is installed but not active, source it:

```bash
source "$IDF_PATH/export.sh"
# or the path printed by setup-env.sh
```

Do not install ESP-IDF inside the repository. Do not claim the environment is ready until `check-env.sh` passes.

### Non-ASCII paths

This repository may live under a path containing Chinese characters. ESP-IDF 6.0.1 can fail with:

```text
cannot read spec file '.../picolibc.specs'
```

If `check-env.sh` reports a non-ASCII path, build from an ASCII path, for example:

```bash
cp -R /path/to/Flashduck/firmware/voice ~/flashduck-voice-build
cd ~/flashduck-voice-build
```

For DuckClaw, copy `firmware/duckclaw` to an ASCII path. Do not rename the user's repository without asking.

`build-voice.sh` and `build-duckclaw.sh` do this automatically with `prepare-ascii-build.sh`; their output paths are printed.

---

## 1. Decide which firmware the user needs

Ask one short question if it is not clear:

| User wants | Firmware | Start here |
|---|---|---|
| Voice chat, wake word, eye expressions, gait tuning, MCP tools | `firmware/voice` | `references/voice-firmware.md` |
| DingTalk, local memory, emotions, relationship growth, AI poses | `firmware/duckclaw` | `references/duckclaw.md` |
| Both | Explain they are mutually exclusive | `docs/architecture.md` |
| Only hardware / shell / wiring | No firmware change | `references/hardware.md` |

**They cannot run at the same time.** Flashing one replaces the other.

---

## 2. Hardware safety rules

Read `references/hardware.md` before changing pins or actions.

- Mouth servo: **45–60° only**. Never command more than 60°.
- Neck servo: **45–180°**, 135° is forward.
- Legs/feet: 90° neutral; keep normal poses near 75–105°.
- Do not change GPIOs unless the user confirms their board revision.
- Power off before unplugging servos. Do not back-drive a powered servo.
- A successful build is not hardware validation. State what still needs a real robot.

---

## 3. Work at the smallest correct layer

### Voice firmware

Repository root for commands: `firmware/voice`.

| Task | Owning files |
|---|---|
| Eye color/shape/state | `main/boards/flashduck/flashduck_display.cc/.h` |
| Predefined actions | `main/boards/flashduck/flashduck_controller.cc/.h` |
| Motion primitives | `main/boards/flashduck/flashduck_movements.cc/.h` |
| Web gait page | `main/boards/flashduck/flashduck_web_server.cc` |
| Camera/MCP tools | `main/mcp_server.cc`, board `GetCamera()` |
| Board pins | `main/boards/flashduck/config.h` |
| Build entry | `scripts/build.py` |

Build:

```bash
cd firmware/voice
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
```

Full image: `build/merged-binary.bin` at `0x0`.

### DuckClaw

Repository root for commands: `firmware/duckclaw`.

| Task | Owning files |
|---|---|
| Predefined duck actions | `main/tools/tool_otto.c` |
| Motion/pose primitives | `main/otto/otto_movements.c/.h` |
| Board pins | `main/board/board_config.h` |
| Agent prompt/tools | `spiffs_data/config/AGENTS.md`, `TOOLS.md`, `SOUL.md` |
| DingTalk/on-device agent | `main/dingtalk/`, `main/agent/` |
| LCD UI | `main/lcd/lcd_display.c` |

Build:

```bash
cd firmware/duckclaw
idf.py set-target esp32s3
idf.py build
idf.py merge-bin
```

Full image: `build/merged-binary.bin` at `0x0`.

DuckClaw is **CC BY-NC-SA 4.0**; do not copy it into the MIT voice firmware without checking the license.

---

## 4. Beginner mode

When the user says they are new:

1. Explain the two firmware choices in one short paragraph.
2. Check the environment before asking them to install anything.
3. Give one command at a time, with the expected result.
4. After flashing, explain:
   - how to find the serial port;
   - what success looks like in the log;
   - how to open the config page / web gait page.
5. For a first change, prefer a safe, visible recipe:
   - change an eye color;
   - add a new duck action;
   - change a gait default;
   - change a DuckClaw prompt response.
6. Never start with pin changes, servo trimming, or partition changes.

Use `references/recipes.md` for step-by-step recipes.

---

## 5. Verification and reporting

Before saying “done”:

- run the relevant build;
- run host tests when the voice project changes:

```bash
cd firmware/voice
python3 -m unittest discover -s scripts/tests -v
```

- check the generated image exists;
- report:
  - what changed;
  - what was built/tested;
  - what still needs real hardware;
  - any license or partition caveat.

Do not say “works on hardware” from a successful compile.

---

## References

- `references/environment.md` — install/check ESP-IDF, Python, CMake, Ninja, esptool, serial ports, non-ASCII path.
- `references/hardware.md` — pins, servos, display, camera, audio, power, limits.
- `references/voice-firmware.md` — voice code map, eye/action/gait/MCP/camera recipes.
- `references/duckclaw.md` — DuckClaw code map, actions, prompts, config, license.
- `references/recipes.md` — task-oriented beginner recipes.
- `docs/architecture.md` — project and code logic.
- `docs/coding-agent.md` — decision tree, guardrails, hand-off format.
