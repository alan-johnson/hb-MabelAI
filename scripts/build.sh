#!/usr/bin/env bash
# build.sh — production vs emulator/phone build helper
#
# Usage:
#   scripts/build.sh prod                  — clean store build (no API keys)
#   scripts/build.sh emulator [platform]   — build + install on emulator (default: basalt)
#   scripts/build.sh phone <ip>            — build + install on phone via IP
#
# Platforms: basalt  chalk  diorite  emery  flint
#
# For emulator and phone builds, HB_* environment variables are used to bake
# in your API keys.  Easiest way: create a .env file (gitignored) and source it:
#
#   source .env && scripts/build.sh emulator basalt
#   source .env && scripts/build.sh phone 192.168.1.42
#
# Example .env:
#   export HB_PROVIDER=claude
#   export HB_CLAUDE_API_KEY=sk-ant-api03-...
#   export HB_MODEL=claude-haiku-4-5-20251001

set -euo pipefail

MODE="${1:-help}"

# ---- helpers ----------------------------------------------------------------

gen_env() {
  echo "  → Generating env-defaults.js..."
  npm run gen-env --silent
}

build() {
  echo "  → Building..."
  pebble build
}

# Remove the source map from the .pbw (debug-only artifact; saves ~52 KB).
# Uses Python's zipfile to rewrite the archive atomically — avoids the race
# where pebble's bundler finalises the .pbw after `pebble build` returns.
strip_sourcemap() {
  local pbw="build/hb-MabelAI.pbw"
  [ -f "$pbw" ] || return 0
  if python3 - "$pbw" << 'PYEOF'
import sys, zipfile, os
MAP = 'pebble-js-app.js.map'
pbw = sys.argv[1]
with zipfile.ZipFile(pbw) as src:
    if MAP not in src.namelist():
        sys.exit(1)          # nothing to strip — signal caller via exit code
    tmp = pbw + '.tmp'
    with zipfile.ZipFile(tmp, 'w', zipfile.ZIP_DEFLATED) as dst:
        for item in src.infolist():
            if item.filename != MAP:
                dst.writestr(item, src.read(item.filename))
os.replace(tmp, pbw)         # atomic swap
PYEOF
  then
    echo "  → Source map stripped"
  fi
}

# Minify the bundled JS inside the .pbw using terser (prod builds only).
# Drops console.log calls and mangles variable names — saves ~12-15 KB.
# Dev builds keep the full JS so `pebble logs` output remains readable.
minify_js() {
  local pbw="build/hb-MabelAI.pbw"
  if [ ! -f "$pbw" ]; then return; fi
  if ! command -v npx &>/dev/null; then
    echo "  → npx not found — skipping JS minification"
    return
  fi

  local tmpdir
  tmpdir=$(mktemp -d)
  trap 'rm -rf "$tmpdir"' RETURN

  unzip -q -o "$pbw" pebble-js-app.js -d "$tmpdir" 2>/dev/null || return

  local orig new pbwabs
  orig=$(wc -c < "$tmpdir/pebble-js-app.js" | tr -d ' ')
  pbwabs=$(cd "$(dirname "$pbw")" && pwd)/$(basename "$pbw")

  if npx terser \
      --compress "drop_console=true" \
      --mangle \
      --comments false \
      "$tmpdir/pebble-js-app.js" \
      -o "$tmpdir/pebble-js-app.min.js" 2>/dev/null; then
    mv "$tmpdir/pebble-js-app.min.js" "$tmpdir/pebble-js-app.js"
    (cd "$tmpdir" && zip -q -j "$pbwabs" pebble-js-app.js)
    new=$(wc -c < "$tmpdir/pebble-js-app.js" | tr -d ' ')
    echo "  → JS minified: $orig → $new bytes (saved $((orig - new)) bytes)"
  else
    echo "  → JS minification failed — keeping original"
  fi

}

# ---- modes ------------------------------------------------------------------

case "$MODE" in

  prod|production)
    echo "=== Production build (no API keys) ==="
    # Explicitly clear all HB_* vars so gen-env writes empty strings
    unset HB_PROVIDER HB_CLAUDE_API_KEY HB_GEMINI_API_KEY \
          HB_CHATGPT_API_KEY HB_MODEL HB_MAX_TOKENS HB_MAX_HISTORY
    gen_env
    build
    minify_js
    # Flush any pending writes from pebble's bundler subprocess, then strip.
    sync
    strip_sourcemap
    pbw_size=$(wc -c < "build/hb-MabelAI.pbw" | tr -d ' ')
    echo "  → Final .pbw: ${pbw_size} bytes ($((pbw_size / 1024)) KB)"
    echo ""
    echo "Done.  Upload to the Rebble store:"
    echo "  build/hb-MabelAI.pbw"
    ;;

  emulator|emu)
    PLATFORM="${2:-basalt}"
    echo "=== Emulator build — platform: $PLATFORM ==="
    gen_env
    build
    PBW_SIZE=$(wc -c < "build/hb-MabelAI.pbw" | tr -d ' ')
    echo "  → .pbw: ${PBW_SIZE} bytes ($((PBW_SIZE / 1024)) KB)"
    echo "  → Installing on emulator ($PLATFORM)..."
    pebble install --emulator "$PLATFORM"
    ;;

  phone)
    PHONE_IP="${2:?Error: phone IP required.  Usage: scripts/build.sh phone <IP>}"
    echo "=== Phone build — $PHONE_IP ==="
    gen_env
    build
    PBW_SIZE=$(wc -c < "build/hb-MabelAI.pbw" | tr -d ' ')
    echo "  → .pbw: ${PBW_SIZE} bytes ($((PBW_SIZE / 1024)) KB)"
    echo "  → Installing on phone ($PHONE_IP)..."
    pebble install --phone "$PHONE_IP"
    ;;

  help|*)
    echo "Usage: scripts/build.sh <mode> [option]"
    echo ""
    echo "  prod                  Clean store build — no API keys baked in"
    echo "  emulator [platform]   Build + install on emulator  (default: basalt)"
    echo "  phone <IP>            Build + install on phone via IP address"
    echo ""
    echo "Platforms: basalt  chalk  diorite  emery  flint"
    echo ""
    echo "Set HB_* env vars before running emulator/phone builds, e.g.:"
    echo "  source .env && scripts/build.sh emulator basalt"
    echo "  source .env && scripts/build.sh phone 192.168.1.42"
    ;;

esac
