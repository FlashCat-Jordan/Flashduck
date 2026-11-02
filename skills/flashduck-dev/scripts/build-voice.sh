#!/usr/bin/env bash
# Build the Flashduck voice firmware from an ASCII-friendly path.
set -euo pipefail
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
VOICE="$REPO/firmware/voice"

if LC_ALL=C grep -q '[^ -~]' <<< "$VOICE"; then
  echo "Non-ASCII path detected; preparing an ASCII build copy..." >&2
  VOICE="$(bash "$REPO/skills/flashduck-dev/scripts/prepare-ascii-build.sh" voice | tail -1)"
  echo "Building in: $VOICE" >&2
fi

IDF_DIR="${IDF_PATH:-$HOME/esp/esp-idf-v6.0.1}"
if [[ ! -f "$IDF_DIR/export.sh" ]]; then
  echo "ERROR: ESP-IDF not found at $IDF_DIR. Run check-env.sh/setup-env.sh first." >&2
  exit 1
fi
# shellcheck disable=SC1090
source "$IDF_DIR/export.sh" >/dev/null 2>&1

cd "$VOICE"
python3 scripts/build.py flashduck --name flashduck --language zh-CN \
  --wake-word wn9_xiaoyaxiaoya_tts2
echo
echo "Full image: $VOICE/build/merged-binary.bin"
