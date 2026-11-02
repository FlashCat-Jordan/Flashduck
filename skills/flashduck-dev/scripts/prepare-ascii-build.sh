#!/usr/bin/env bash
# Copy one firmware project to an ASCII-only build directory.
# Usage: bash prepare-ascii-build.sh <voice|duckclaw>
set -euo pipefail
REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
TARGET="${1:-}"
case "$TARGET" in
  voice) SRC="$REPO/firmware/voice" ;;
  duckclaw) SRC="$REPO/firmware/duckclaw" ;;
  *) echo "Usage: $0 <voice|duckclaw>" >&2; exit 2 ;;
esac
BASE="${FLASHDUCK_BUILD_DIR:-$HOME/flashduck-build}"
DEST="$BASE/$TARGET"
mkdir -p "$DEST"
if command -v rsync >/dev/null 2>&1; then
  rsync -a --exclude build --exclude managed_components --exclude sdkconfig --exclude sdkconfig.old "$SRC/" "$DEST/"
else
  (cd "$SRC" && tar --exclude=build --exclude=managed_components --exclude=sdkconfig --exclude=sdkconfig.old -cf - .) | (cd "$DEST" && tar -xf -)
fi
echo "$DEST"
