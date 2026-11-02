# Firmware

Two independent interaction systems, one robot body. The user chooses one; only one runs at a time.

| Interaction system | Directory | Experience | License |
|---|---|---|---|
| **Voice assistant** | [`voice/`](voice/) | Wake word, voice conversation, eye, actions, web gait tuning, MCP/camera | MIT (root `LICENSE`) |
| **DuckClaw** | [`duckclaw/`](duckclaw/) | On-device agent: external tools, DingTalk/social, emotions, relationship, direct servo control | CC BY-NC-SA 4.0 (`duckclaw/LICENSE`) |

They are not meant to run together. Flash the one you want to use. The other one is replaced on the device.

## Build from the correct directory

Voice:

```bash
cd firmware/voice
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
```

DuckClaw:

```bash
cd firmware/duckclaw
idf.py set-target esp32s3
idf.py build
idf.py merge-bin
```

Both full images are flashed at `0x0`. Do not mix the two output images.

## Before building

- Use ESP-IDF **6.0.1+**.
- On macOS, build from an ASCII path; ESP-IDF 6.0.1 can fail on a Chinese path with `picolibc.specs`.
- The skill at [`../skills/flashduck-dev/SKILL.md`](../skills/flashduck-dev/SKILL.md) checks the environment and has beginner recipes.
- Read [`../docs/architecture.md`](../docs/architecture.md) for the code map.
