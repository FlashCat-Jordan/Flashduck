#!/usr/bin/env bash
# Build DuckClaw from an ASCII-friendly path.
set -euo pipefail
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
DUCK="$REPO/firmware/duckclaw"

if LC_ALL=C grep -q '[^ -~]' <<< "$DUCK"; then
  echo "Non-ASCII path detected; preparing an ASCII build copy..." >&2
  DUCK="$(bash "$REPO/skills/flashduck-dev/scripts/prepare-ascii-build.sh" duckclaw | tail -1)"
  echo "Building in: $DUCK" >&2
fi

IDF_DIR="${IDF_PATH:-$HOME/esp/esp-idf-v6.0.1}"
if [[ ! -f "$IDF_DIR/export.sh" ]]; then
  echo "ERROR: ESP-IDF not found at $IDF_DIR. Run check-env.sh/setup-env.sh first." >&2
  exit 1
fi
# shellcheck disable=SC1090
source "$IDF_DIR/export.sh" >/dev/null 2>&1

cd "$DUCK"
idf.py set-target esp32s3
idf.py build
idf.py merge-bin
echo
echo "Full image: $DUCK/build/merged-binary.bin"
