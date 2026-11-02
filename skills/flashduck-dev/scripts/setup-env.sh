#!/usr/bin/env bash
# Install missing host dependencies and ESP-IDF for Flashduck.
# Usage: bash skills/flashduck-dev/scripts/setup-env.sh [--yes] [--idf-path PATH]
set -euo pipefail

YES=0
IDF_DIR="${IDF_PATH:-$HOME/esp/esp-idf-v6.0.1}"

while [[ $# -gt 0 ]]; do
  case "$1" in
    --yes|-y) YES=1; shift ;;
    --idf-path) IDF_DIR="$2"; shift 2 ;;
    -h|--help)
      echo "Usage: $0 [--yes] [--idf-path PATH]"
      exit 0 ;;
    *) echo "Unknown option: $1" >&2; exit 2 ;;
  esac
done

OS="$(uname -s)"
echo "Flashduck setup"
echo "OS: $OS"
echo "ESP-IDF target: $IDF_DIR"
echo

if [[ "$OS" == "Darwin" ]]; then
  if ! xcode-select -p >/dev/null 2>&1; then
    echo "Xcode Command Line Tools are missing."
    echo "Run: xcode-select --install"
    exit 1
  fi
  if ! command -v brew >/dev/null 2>&1; then
    echo "Homebrew is missing."
    echo 'Install it from https://brew.sh, then rerun this script.'
    exit 1
  fi
  PKGS=()
  for p in git python3 cmake ninja dfu-util; do
    if ! brew list "$p" >/dev/null 2>&1; then PKGS+=("$p"); fi
  done
  if [[ ${#PKGS[@]} -gt 0 ]]; then
    echo "Homebrew packages to install: ${PKGS[*]}"
    if [[ "$YES" -ne 1 ]]; then
      echo "Re-run with --yes to install them."
      exit 1
    fi
    brew install "${PKGS[@]}"
  else
    echo "Homebrew prerequisites: OK"
  fi
elif [[ "$OS" == "Linux" ]]; then
  if command -v apt-get >/dev/null 2>&1; then
    echo "Linux packages may be installed with apt-get:"
    echo "  sudo apt-get update"
    echo "  sudo apt-get install -y git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0"
    if [[ "$YES" -ne 1 ]]; then
      echo "Re-run with --yes to install them."
      exit 1
    fi
    sudo apt-get update
    sudo apt-get install -y git wget flex bison gperf python3 python3-pip python3-venv cmake ninja-build ccache libffi-dev libssl-dev dfu-util libusb-1.0-0
  else
    echo "This Linux distribution has no apt-get. Install git, python3, cmake, ninja and dfu-util with your package manager."
    [[ "$YES" -eq 1 ]] || exit 1
  fi
else
  echo "Windows/other OS: use the ESP-IDF installer or WSL2."
  echo "See references/environment.md for manual steps."
  exit 1
fi

if [[ ! -f "$IDF_DIR/export.sh" ]]; then
  echo
  echo "ESP-IDF not found at: $IDF_DIR"
  echo "Cloning ESP-IDF v6.0.1 (this can take a while)..."
  if [[ "$YES" -ne 1 ]]; then
    echo "Re-run with --yes to clone and install ESP-IDF."
    exit 1
  fi
  mkdir -p "$(dirname "$IDF_DIR")"
  git clone -b v6.0.1 --recursive https://github.com/espressif/esp-idf.git "$IDF_DIR"
  (cd "$IDF_DIR" && ./install.sh esp32s3)
else
  echo "ESP-IDF found: $IDF_DIR"
  if ! bash -c "source '$IDF_DIR/export.sh' >/dev/null 2>&1; command -v idf.py >/dev/null 2>&1 && command -v cmake >/dev/null 2>&1 && command -v ninja >/dev/null 2>&1 && command -v esptool >/dev/null 2>&1"; then
    echo "ESP-IDF environment is incomplete (idf.py/cmake/ninja/esptool); running install.sh esp32s3..."
    (cd "$IDF_DIR" && ./install.sh esp32s3)
  fi
fi

echo
echo "Checking installed tools..."
bash -c "source '$IDF_DIR/export.sh' >/dev/null 2>&1; idf.py --version"
bash -c "source '$IDF_DIR/export.sh' >/dev/null 2>&1; command -v cmake ninja esptool"

echo
echo "Done. Activate the environment in every new shell with:"
echo "  source \"$IDF_DIR/export.sh\""
echo
echo "Then run:"
echo "  bash skills/flashduck-dev/scripts/check-env.sh"
