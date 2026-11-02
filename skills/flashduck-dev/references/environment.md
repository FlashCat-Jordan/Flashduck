# Environment Setup

Flashduck uses **ESP-IDF 6.0.1+**. The voice firmware and DuckClaw are separate ESP-IDF projects, but they share the same host toolchain.

## Supported hosts

- macOS (Intel or Apple Silicon)
- Linux (Ubuntu/Debian recommended)
- Windows: use the Espressif ESP-IDF installer or WSL2; the shell scripts in this skill are for macOS/Linux.

## One-command check

```bash
bash skills/flashduck-dev/scripts/check-env.sh
```

The check verifies:

- git, python3 (3.10+);
- ESP-IDF (`IDF_PATH` or `~/esp/esp-idf-v6.0.1`);
- `idf.py` and version >= 6.0.1;
- cmake, ninja and esptool inside the IDF environment;
- repository paths;
- non-ASCII path risk;
- available serial ports.

Exit code `0` means ready (possibly with warnings); `1` means a required dependency is missing.

## Automatic install

Read the plan first, then approve:

```bash
bash skills/flashduck-dev/scripts/setup-env.sh
# after the user approves:
bash skills/flashduck-dev/scripts/setup-env.sh --yes
```

What it does:

- macOS: checks Xcode Command Line Tools/Homebrew; installs git, cmake, ninja, dfu-util if needed.
- Linux: prints the apt command; with `--yes`, installs git, Python, CMake, Ninja, ccache and USB tools.
- Clones ESP-IDF **v6.0.1** to `~/esp/esp-idf-v6.0.1` if missing.
- Runs `install.sh esp32s3`.
- Prints the export command.

The script never edits the repository and never writes secrets.

## Manual install

### macOS

```bash
xcode-select --install
brew install git python3 cmake ninja dfu-util

git clone -b v6.0.1 --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf-v6.0.1
cd ~/esp/esp-idf-v6.0.1
./install.sh esp32s3
source export.sh
idf.py --version
```

### Ubuntu/Debian

```bash
sudo apt-get update
sudo apt-get install -y git wget flex bison gperf python3 python3-pip python3-venv \
  cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0

git clone -b v6.0.1 --recursive https://github.com/espressif/esp-idf.git ~/esp/esp-idf-v6.0.1
cd ~/esp/esp-idf-v6.0.1
./install.sh esp32s3
source export.sh
idf.py --version
```

### Windows

Use the ESP-IDF installer from Espressif, select ESP32-S3, then use the "ESP-IDF PowerShell" or "ESP-IDF Command Prompt" shortcut. WSL2 also works; USB passthrough is required for flashing.

## Activate the environment

Every new terminal needs:

```bash
source ~/esp/esp-idf-v6.0.1/export.sh
```

Check:

```bash
idf.py --version
```

## Non-ASCII repository paths

ESP-IDF 6.0.1 can fail when the project path contains Chinese characters:

```text
xtensa-esp-elf-gcc: fatal error: cannot read spec file '.../picolibc.specs'
```

Build from an ASCII path:

```bash
mkdir -p ~/flashduck-build
cp -R firmware/voice ~/flashduck-build/voice
cd ~/flashduck-build/voice
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
```

DuckClaw is the same; copy `firmware/duckclaw` to an ASCII path, or let `build-duckclaw.sh` call `prepare-ascii-build.sh` automatically.

## Common problems

| Symptom | Cause | Fix |
|---|---|---|
| `idf.py: command not found` | Environment not sourced | `source ~/esp/esp-idf-v6.0.1/export.sh` |
| `ESP-IDF v5.x` | Too old | Install v6.0.1+ |
| `picolibc.specs` or `\x0a` in a path | Non-ASCII project path | Build from ASCII path |
| `CMake Error: CMAKE_HOME_DIRECTORY` | Build directory from another path | Remove `build/` or `idf.py fullclean` |
| `Permission denied` on `/dev/ttyUSB0` | Linux serial permissions | `sudo usermod -aG dialout $USER`, then log in again |
| `esptool` not found | IDF Python env not active | Source `export.sh`; rerun `install.sh esp32s3` |
| Component download fails | Network/registry problem | Retry; use a mirror or an existing `managed_components/` |

## Safety

- Do not install ESP-IDF inside the repository.
- Do not commit `sdkconfig`, `build/`, `managed_components/`, or secrets.
- Do not use `sudo` for the build itself.
- Ask before large downloads or package installation.
