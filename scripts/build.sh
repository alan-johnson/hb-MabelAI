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
  echo "  → Cleaning previous builds..."
  rm -f build/hb-MabelAI.pbw build/pebble-js-app.js build/pebble-js-app.js.map 2>/dev/null || true
  echo "  → Building..."
  pebble build
}

# Optimise the .pbw in a single Python pass:
#   • strips pebble-js-app.js.map (saves ~52 KB)
#   • minifies pebble-js-app.js via terser if available (saves ~12-15 KB)
#   • forces every entry to STORED (compress_type=0) — required by Pebble JS runtime
#
# Doing both operations in one Python rewrite avoids the flag_bits incompatibility
# that occurs when InfoZIP's `zip` command sets data-descriptor bits that Python's
# zipfile module then copies verbatim without writing the matching descriptors.
optimize_pbw() {
  local pbw="build/hb-MabelAI.pbw"
  [ -f "$pbw" ] || return 0

  # Optionally minify pebble-js-app.js with terser before the Python rewrite.
  local minified_js=""
  if command -v npx &>/dev/null; then
    local tmpdir
    tmpdir=$(mktemp -d)
    trap 'rm -rf "$tmpdir"' RETURN
    if unzip -q -o "$pbw" pebble-js-app.js -d "$tmpdir" 2>/dev/null; then
      local orig
      orig=$(wc -c < "$tmpdir/pebble-js-app.js" | tr -d ' ')
      if npx terser \
          --compress "drop_console=true" \
          --mangle \
          --comments false \
          "$tmpdir/pebble-js-app.js" \
          -o "$tmpdir/pebble-js-app.min.js" 2>/dev/null; then
        local new
        new=$(wc -c < "$tmpdir/pebble-js-app.min.js" | tr -d ' ')
        minified_js="$tmpdir/pebble-js-app.min.js"
        echo "  → JS minified: $orig → $new bytes (saved $((orig - new)) bytes)"
      else
        echo "  → JS minification failed — keeping original"
      fi
    fi
  fi

  # Single Python pass: strip source map, inject minified JS (if any), force STORED.
  if python3 - "$pbw" "$minified_js" << 'PYEOF'
import sys, zipfile, os

MAP   = 'pebble-js-app.js.map'
JSKEY = 'pebble-js-app.js'

pbw         = sys.argv[1]
minified_js = sys.argv[2] if len(sys.argv) > 2 else ''

with zipfile.ZipFile(pbw) as src:
    tmp = pbw + '.tmp'
    with zipfile.ZipFile(tmp, 'w') as dst:
        for item in src.infolist():
            if item.filename == MAP:
                continue          # drop source map
            item.compress_type = 0  # STORED — Pebble runtime requires uncompressed entries
            if item.filename == JSKEY and minified_js:
                with open(minified_js, 'rb') as fh:
                    data = fh.read()
            else:
                data = src.read(item.filename)
            dst.writestr(item, data)
os.replace(tmp, pbw)              # atomic swap
PYEOF
  then
    echo "  → Source map stripped"
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
    # Flush any pending writes from pebble's bundler subprocess, then optimise.
    sync
    optimize_pbw
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
