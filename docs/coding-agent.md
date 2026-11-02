# Coding Agent Workflow

This page is the **agent logic** layer. It is not a firmware manual; it tells an agent how to choose the right subsystem and avoid damaging the robot or the repository.

## 1. Read order

1. `README.md` — what the product is and which system the user wants.
2. `docs/architecture.md` — product/code boundaries.
3. `skills/flashduck-dev/SKILL.md` — environment, safety, recipes.
4. The owning board/code file before editing.
5. The closest existing implementation before adding a new pattern.

Do not start by rewriting the architecture or moving files.

## 2. Decision tree

```text
User request
  |
  +-- "check my computer / install / build / flash"
  |      -> skill scripts/check-env.sh
  |      -> if missing: references/environment.md + setup-env.sh (with consent)
  |
  +-- "voice / wake word / eye / gait / MCP / camera"
  |      -> firmware/voice
  |
  +-- "DingTalk / on-device agent / emotion / relationship / pose"
  |      -> firmware/duckclaw
  |
  +-- "shell / wiring / servo direction / 3D print"
  |      -> hardware reference; no firmware change unless pins change
  |
  +-- "I want a new feature"
         -> identify the owning layer
         -> smallest code path
         -> build + tests
         -> flash instructions
```

If the user is not sure, ask one question: **“你想要会说话的语音版，还是安静发钉钉的 DuckClaw？”**

## 3. Three levels of reasoning

### Project logic

- One robot, two mutually exclusive firmware systems.
- Shared hardware: six servos, display, optional camera, audio, battery.
- The product README owns the story; do not duplicate it in code docs.

### Code logic

- Voice: board init -> display/eye + audio + protocol -> MCP -> controller -> movements.
- DuckClaw: DingTalk -> agent loop -> tools -> servo/LCD.
- A feature should live at the owning layer:
  - eye style in display;
  - action registry in controller;
  - motion math in movements;
  - board pin in `config.h`;
  - model-facing name in MCP schema and prompt docs.
- If a change crosses layers, update all of them together: enum + switch + schema + prompts.

### Agent logic

- Check environment before build.
- Read before write.
- Prefer one small patch over a refactor.
- Build after code changes.
- Report limitations and hardware steps.
- Never claim hardware validation from a compile.

## 4. Safety rules

- Mouth **45–60°**, neck **45–180°**.
- Do not expose raw GPIO or unlimited servo values to an LLM tool.
- Do not change existing board pins silently; add a variant.
- Do not put secrets in source, prompts, README or commits.
- Do not commit `build/`, `managed_components/`, `sdkconfig`, `dependencies.lock` or generated assets.
- Do not mix MIT voice code and CC BY-NC-SA DuckClaw code.
- Ask before installing large dependencies or changing partitions.

## 5. Environment logic

The skill must check before acting:

```bash
bash skills/flashduck-dev/scripts/check-env.sh
```

If missing:

1. Read `references/environment.md`.
2. Explain what is missing in one or two lines.
3. Ask for approval.
4. Run `setup-env.sh --yes`.
5. Source `export.sh`.
6. Re-run `check-env.sh`.

If the repository path is non-ASCII, build from an ASCII copy. `build-voice.sh` and `build-duckclaw.sh` handle this automatically with `prepare-ascii-build.sh`.

## 6. Verification matrix

| Change | Verify |
|---|---|
| Voice UI/eye | build voice; check image exists |
| Voice action/gait | build voice; run host tests; physical servo limits still need a robot |
| Voice MCP tool | build; call the tool through a client if possible |
| DuckClaw action | build DuckClaw; merge; check app partition size |
| DuckClaw prompt | rebuild (SPIFFS is in the image); verify prompt file in image |
| Board pins | build; hardware check mandatory |
| Docs/skill only | no firmware build required; check links and commands |

Commands:

```bash
cd firmware/voice
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
python3 -m unittest discover -s scripts/tests -v

cd ../duckclaw
idf.py set-target esp32s3
idf.py build
idf.py merge-bin
```

## 7. Hand-off format

When finished, report:

```text
Changed:
- file: what and why

Verified:
- command: result
- image: path/size

Still needs hardware:
- servo direction / mouth limit / display / camera / audio / battery

Caveats:
- license, partition, backend, or config
```

## 8. Anti-patterns

- Adding a second task loop for one action.
- Editing `build/` or generated assets.
- Putting board-specific code into `main/application.*`.
- Adding an LLM tool before the controller supports the action.
- Assuming DuckClaw can see through the camera.
- Using the voice firmware's build script for DuckClaw.
- Saying “done” without a build or without stating hardware checks.

## 9. Beginner interaction

For a 0-experience user:

1. Check environment for them.
2. Ask them to choose a safe first change (eye color / duck action / prompt).
3. Give one command block at a time.
4. Explain the expected output.
5. Ask for a serial log or video before diagnosing hardware.

Do not start with pin changes, servo trims, partition edits or camera drivers.
