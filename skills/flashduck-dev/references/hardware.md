# Flashduck Hardware Reference

## Board

- MCU: ESP32-S3, 16 MB flash board (verify the actual module)
- Display: ST7789 240×240, SPI
- Servo outputs: 6
- Camera: optional OV2640 / OV3660 / GC2145 on the camera board
- Audio: I2S microphone and speaker
- Power: rechargeable Li-ion battery with charge detection

## Servo map (Flashduck)

| Joint | GPIO | Notes |
|---|---:|---|
| Left leg | 5 | forward/back swing |
| Right leg | 43 | forward/back swing |
| Left foot | 6 | lift |
| Right foot | 44 | lift |
| Mouth | 4 | former left-hand output; **45–60° only** |
| Neck | 7 | former right-hand output; **45–180°** |

## Display / I2C / audio pins

From `firmware/voice/main/boards/flashduck/config.h`:

| Signal | GPIO |
|---|---:|
| LCD backlight | 38 |
| LCD MOSI | 45 |
| LCD clock | 48 |
| LCD DC | 47 |
| LCD reset | 1 |
| LCD CS | not used |
| I2C SDA | 15 |
| I2C SCL | 16 |
| Audio WS | 40 |
| Audio BCLK | 42 |
| Audio DIN | 41 |
| Audio DOUT | 39 |
| Charge detect (non-camera board) | 21 |

On the camera board, GPIO21 is used by the camera; the firmware uses USB/voltage detection instead.

## Safe ranges

- Mouth: **45° closed, 60° fully open**. Never exceed 60°.
- Neck: **45° left, 135° forward, 180° right**.
- Legs/feet: **90° neutral**; normal walking stays near 80–100°.
- Do not set arbitrary 0–180° values for mouth/neck in user-facing tools.
- Power off before changing servo wiring.
- Do not force a powered servo by hand.

## Camera

- Optional; boot continues without it.
- Voice firmware auto-detects OV2640 / OV3660 / GC2145; the camera MCP tool is only registered when a camera exists.
- Camera photo explanation depends on the backend/vision endpoint.
- DuckClaw currently does **not** read camera images; it uses the camera-board pin layout only.

## Board variants

- Flashduck voice firmware and DuckClaw in this repository target the **Flashduck camera-board routing**.
- Do not flash a camera-board image onto a non-camera board, or vice versa.
- Pin changes require a new board variant, not a silent edit to an existing board.

## Hardware validation

A successful build only proves that the code compiled. Still check:

- servo direction and trim;
- mouth/neck limits before installing the shell;
- display orientation/backlight;
- camera detection (if installed);
- speaker/microphone;
- battery/charging;
- Wi-Fi provisioning.
