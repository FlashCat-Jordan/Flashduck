#!/usr/bin/env bash
# Check the host environment for Flashduck development.
# Usage: bash skills/flashduck-dev/scripts/check-env.sh
set -uo pipefail

REPO="$(cd "$(dirname "${BASH_SOURCE[0]}")/../../.." && pwd)"
FAIL=0
WARN=0

ok()   { printf '  [OK]   %s\n' "$*"; }
miss() { printf '  [MISS] %s\n' "$*"; FAIL=1; }
warn() { printf '  [WARN] %s\n' "$*"; WARN=1; }

printf 'Flashduck environment check\n'
printf 'Repository: %s\n\n' "$REPO"

printf 'Basic tools\n'
for cmd in git python3; do
  if command -v "$cmd" >/dev/null 2>&1; then
    ok "$cmd: $(command -v "$cmd")"
  else
    miss "$cmd is not installed"
  fi
done
for cmd in cmake ninja; do
  if command -v "$cmd" >/dev/null 2>&1; then ok "$cmd: $(command -v "$cmd")"; fi
done

printf '\nPython version\n'
if command -v python3 >/dev/null 2>&1; then
  PYVER="$(python3 -c 'import sys; print("%d.%d.%d" % sys.version_info[:3])' 2>/dev/null || echo unknown)"
  if python3 -c 'import sys; sys.exit(0 if sys.version_info >= (3, 10) else 1)' 2>/dev/null; then
    ok "Python $PYVER"
  else
    miss "Python $PYVER is too old (need 3.10+)"
  fi
else
  miss "python3 is not available"
fi

printf '\nESP-IDF\n'
IDF_DIR=""
if [[ -n "${IDF_PATH:-}" && -f "${IDF_PATH}/export.sh" ]]; then
  IDF_DIR="$IDF_PATH"
else
  for candidate in \
    "$HOME/esp/esp-idf-v6.0.1" \
    "$HOME/esp/esp-idf" \
    "$HOME/esp/esp-idf-v6.1" \
    "/opt/esp-idf" \
    "/opt/esp/esp-idf"; do
    if [[ -f "$candidate/export.sh" ]]; then IDF_DIR="$candidate"; break; fi
  done
fi

if [[ -z "$IDF_DIR" ]]; then
  miss "ESP-IDF not found (set IDF_PATH or install it; see references/environment.md)"
else
  ok "ESP-IDF: $IDF_DIR"
  IDF_VER="$(bash -c "source '$IDF_DIR/export.sh' >/dev/null 2>&1; idf.py --version 2>/dev/null" | sed -nE 's/.*v([0-9]+\.[0-9]+\.[0-9]+).*/\1/p' | head -1)"
  if [[ -n "$IDF_VER" ]]; then
    ok "idf.py version: $IDF_VER"
    if printf '%s\n6.0.1\n' "$IDF_VER" | sort -V | head -1 | grep -qx '6.0.1'; then
      ok "ESP-IDF >= 6.0.1"
    else
      miss "ESP-IDF $IDF_VER is too old (need 6.0.1+)"
    fi
  else
    miss "could not run idf.py through $IDF_DIR/export.sh"
  fi
  for cmd in cmake ninja esptool; do
    if bash -c "source '$IDF_DIR/export.sh' >/dev/null 2>&1; command -v $cmd >/dev/null 2>&1"; then
      ok "$cmd available inside the ESP-IDF environment"
    else
      miss "$cmd not found inside the ESP-IDF environment (run install.sh esp32s3)"
    fi
  done
  printf '  Export with: source "%s/export.sh"\n' "$IDF_DIR"
fi

printf '\nProject paths\n'
for p in "firmware/voice/CMakeLists.txt" "firmware/duckclaw/CMakeLists.txt" "skills/flashduck-dev/SKILL.md"; do
  if [[ -f "$REPO/$p" ]]; then ok "$p"; else miss "$p"; fi
done

printf '\nPath and serial checks\n'
if LC_ALL=C grep -q '[^ -~]' <<< "$REPO"; then
  warn "repository path contains non-ASCII characters; ESP-IDF 6.0.1 builds may fail with picolibc.specs"
  warn "copy firmware/voice or firmware/duckclaw to an ASCII path before building"
else
  ok "repository path is ASCII"
fi

PORTS="$(ls /dev/cu.* /dev/ttyUSB* /dev/ttyACM* 2>/dev/null | head -5 || true)"
if [[ -n "$PORTS" ]]; then
  printf '  [INFO] serial candidates:\n%s\n' "$PORTS"
else
  warn "no /dev/cu.*, /dev/ttyUSB* or /dev/ttyACM* device found (connect the board with a data cable)"
fi

printf '\nResult: '
if [[ "$FAIL" -gt 0 ]]; then
  printf 'MISSING dependencies — read references/environment.md and run scripts/setup-env.sh --yes after approval.\n'
  exit 1
fi
if [[ "$WARN" -gt 0 ]]; then
  printf 'READY WITH WARNINGS — review the WARN lines before building.\n'
  exit 0
fi
printf 'READY\n'
