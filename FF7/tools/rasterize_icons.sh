#!/usr/bin/env bash
# Rasterize every SVG in tools/icons/svg/ to a PNG in tools/icons/png/ via Inkscape CLI.
# Usage:   ./tools/rasterize_icons.sh
# Options: SIZE=128 ./tools/rasterize_icons.sh          (default 64)
#          INKSCAPE=/path/to/inkscape.exe ./...         (override binary path)

set -euo pipefail

INKSCAPE="${INKSCAPE:-/c/Program Files/Inkscape/bin/inkscape.exe}"
SIZE="${SIZE:-64}"

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
SRC_DIR="$SCRIPT_DIR/icons/svg"
DST_DIR="$SCRIPT_DIR/icons/png"

if [[ ! -x "$INKSCAPE" ]]; then
  echo "Inkscape not found at: $INKSCAPE" >&2
  echo "Set the INKSCAPE env var to override." >&2
  exit 1
fi

if [[ ! -d "$SRC_DIR" ]]; then
  echo "Source dir missing: $SRC_DIR" >&2
  exit 1
fi

mkdir -p "$DST_DIR"

shopt -s nullglob
svgs=("$SRC_DIR"/*.svg)
if (( ${#svgs[@]} == 0 )); then
  echo "No SVGs found in $SRC_DIR"
  exit 0
fi

count=0
for svg in "${svgs[@]}"; do
  name="$(basename "$svg" .svg)"
  png="$DST_DIR/$name.png"
  "$INKSCAPE" \
    --export-type=png \
    --export-width="$SIZE" \
    --export-height="$SIZE" \
    --export-filename="$png" \
    "$svg" > /dev/null
  echo "  $name.svg -> $name.png"
  count=$((count + 1))
done

echo "Rasterized $count icon(s) at ${SIZE}x${SIZE} -> $DST_DIR"
