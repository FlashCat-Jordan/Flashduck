# Contributing to Flashduck

Thanks for building, printing, flashing or breaking something. This project is
hardware-first: a contribution does not have to be C++.

The repository has two firmware systems: the MIT-licensed voice assistant in the
root and the CC BY-NC-SA 4.0 DuckClaw under `duckclaw/`. Do not move code between
them without checking the licenses.

## Ways to contribute

### 1. 3D printing / hardware

- Remix the shell for a duck, bird, penguin or your own character.
- Improve the neck/mouth linkage, foot grip, cable routing or camera mount.
- Share the print profile, material, weight and any changed screws you used.

### 2. Actions and eye states

- Add a new action in `main/boards/flashduck/`.
- Add an eye state or tune the existing eyelid/pupil curves.
- Keep all animation inside the eye circle; the rest of the screen may be
  physically covered by the shell.

### 3. Firmware and docs

- Web tuning page, NVS parameters, MCP tools, board adapters or wiring docs.
- Fix a bug with a clear reproduction and the board/hardware revision.

## Before opening a PR

```bash
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
python3 -m unittest discover -s scripts/tests -v
```

- Build Flashduck if you touched shared code, Kconfig, CMake, display or audio.
- If you touched the wake word, prompt sounds or assets, say so explicitly:
  the assets partition must be flashed too.
- Format touched C/C++ files with the repository `.clang-format`.
- Do not edit generated files (`build/`, `managed_components/`,
  `main/assets/lang_config.h`, generated mmap headers).
- Do not add a new board unless it is part of the Flashduck scope.

## Hardware validation

A successful compile is not a working robot. In the PR, state what you tested:

- [ ] Both legs and both feet move in the expected direction
- [ ] Neck and mouth move without hitting the shell
- [ ] Camera is detected, or boot continues without one
- [ ] Speaker/microphone work
- [ ] Battery and charging state look reasonable
- [ ] Wi-Fi provisioning and web tuning work
- [ ] Wake word works after a full image flash

## Showcase

If you built one, open a **Showcase** issue with photos, a video link, the
printed material and any parameters you changed. Real builds and real issues
are the best signal for other makers.

## License

By contributing, you agree your contribution is licensed under the repository's
MIT License. Hardware and 3D model files follow their own published licenses.
