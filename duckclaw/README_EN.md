# DuckClaw on Flashduck

This is the second interaction system for **Flashduck**: **DuckClaw**.

It is a separate firmware from the voice assistant in the repository root:

- **It does not speak**: you talk to it through DingTalk messages;
- **Local agent**: C / FreeRTOS, with local memory, sessions and skills;
- **22 emotions**: happy, shy, thinking, surprised, bored, and more;
- **Relationship growth**: from stranger to close companion; hearts on screen;
- **AI-controlled servos**: the model chooses all six joint angles;
- **Captive portal provisioning**: configure Wi-Fi, DingTalk and model keys in a browser.

> The two systems **cannot run at the same time**. Flashing DuckClaw replaces the
> voice firmware; flash the voice firmware again to switch back.

## Hardware adaptation

DuckClaw uses the official camera-board routing on Flashduck:

| Joint | GPIO | Notes |
|---|---|---|
| Left leg | 5 | forward/back swing |
| Right leg | 43 | forward/back swing |
| Left foot | 6 | lift |
| Right foot | 44 | lift |
| Mouth | 4 | former left-hand servo; **45–60°** |
| Neck | 7 | former right-hand servo; **45–180°**, 135° = forward |

- Display: ST7789 240×240 over SPI3;
- BOOT: GPIO0;
- Legs and feet use 90° as neutral;
- Mouth requests above 60° are clamped to protect the mechanism;
- The old humanoid arm actions are replaced by duck actions: head turn, head
  shake, look around, quack, chatter, happy waddle, preen and peck.

## Actions

`self.otto.action`:

`walk`, `walk_backward`, `turn`, `jump`, `swing`, `updown`, `sit`,
`head_turn`, `head_shake`, `look_around`, `quack`, `chatter`,
`happy_waddle`, `preen`, `peck`, `stretch`, `home`

`self.otto.pose` controls six joints directly:

- Legs/feet: 0–180, 90 neutral;
- Mouth: 45–60;
- Neck: 45–180, 135 forward;
- Transition: 100–3000 ms.

## Configuration

1. On first boot (or holding BOOT at power-on), join the `DuckClaw-XXXX` hotspot;
2. Open the captive portal in a browser;
3. Fill in Wi-Fi, DingTalk bot and model API details;
4. Save; the device reboots and joins the network.

For fixed build-time secrets, copy `main/duckclaw_secrets.h.example` to
`main/duckclaw_secrets.h`; the file is gitignored.

## Build and flash

Requires ESP-IDF **6.0.1+**:

```bash
cd duckclaw
idf.py set-target esp32s3
idf.py build
idf.py -p <PORT> flash
```

`partitions.csv` and `spiffs_data/` are included. SPIFFS contains the agent
prompts, web configuration pages and memory files.

## License

The `duckclaw/` directory keeps the upstream **CC BY-NC-SA 4.0** license; see
[LICENSE](LICENSE). The voice firmware in the repository root is MIT. The two
licenses are different; check both before commercial use or redistribution.

Upstream: <https://github.com/FlashCat-Jordan/DuckClaw>
