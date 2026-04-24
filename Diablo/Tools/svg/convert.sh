#!/usr/bin/env bash
# SVG -> PNG via Inkscape. Usage: ./convert.sh <in.svg> <out.png> [dpi]
set -euo pipefail

INKSCAPE="C:/Program Files/Inkscape/bin/inkscape.exe"
IN="${1:?usage: convert.sh <in.svg> <out.png> [dpi]}"
OUT="${2:?usage: convert.sh <in.svg> <out.png> [dpi]}"
DPI="${3:-96}"

mkdir -p "$(dirname "$OUT")"
"$INKSCAPE" --export-type=png --export-dpi="$DPI" --export-filename="$OUT" "$IN"
echo "[convert] $IN -> $OUT @ ${DPI}dpi"
