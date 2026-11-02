#!/usr/bin/env bash
# Flash a full merged image at 0x0.
# Usage: bash flash.sh <PORT> [merged-binary.bin]
set -euo pipefail
PORT="${1:-}"
BIN="${2:-}"
if [[ -z "$PORT" || -z "$BIN" ]]; then
  echo "Usage: bash flash.sh <PORT> <merged-binary.bin>" >&2
  exit 2
fi
if [[ ! -f "$BIN" ]]; then
  echo "ERROR: file not found: $BIN" >&2
  exit 1
fi
IDF_DIR="${IDF_PATH:-$HOME/esp/esp-idf-v6.0.1}"
if [[ -f "$IDF_DIR/export.sh" ]]; then
  # shellcheck disable=SC1090
  source "$IDF_DIR/export.sh" >/dev/null 2>&1
fi
esptool --chip esp32s3 -p "$PORT" -b 460800 write_flash 0x0 "$BIN"
