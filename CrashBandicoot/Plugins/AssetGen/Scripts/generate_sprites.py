"""
Procedural sprite/icon generator for UE5 placeholder textures.

Generates SVG files and converts them to PNG via Inkscape.
Source SVGs are the source of truth — edit this script to change art.

Usage:
    python generate_sprites.py [output_dir] [sprite_filter] [--size=256]

    output_dir    — directory for output files (default: current directory)
    sprite_filter — comma-separated list of sprite names to generate (default: all)
    --size=N      — export resolution in pixels, square (default: 256)

Output: one SPR_<Name>.svg + one SPR_<Name>.png per sprite.

Requires Inkscape on PATH or at the configured location.
"""

import os
import subprocess
import sys

INKSCAPE = r"C:\Program Files\Inkscape\bin\inkscape.exe"
DEFAULT_SIZE = 256


# ---------------------------------------------------------------------------
# SVG helpers
# ---------------------------------------------------------------------------

def svg_doc(width, height, content):
    """Wrap content in an SVG document."""
    return (
        f'<svg xmlns="http://www.w3.org/2000/svg" '
        f'viewBox="0 0 {width} {height}" width="{width}" height="{height}">\n'
        f'{content}'
        f'</svg>\n'
    )


def svg_rect(x, y, w, h, fill, rx=0, opacity=1.0):
    attrs = f'x="{x}" y="{y}" width="{w}" height="{h}" fill="{fill}" rx="{rx}"'
    if opacity < 1.0:
        attrs += f' opacity="{opacity}"'
    return f'  <rect {attrs}/>\n'


def svg_circle(cx, cy, r, fill, opacity=1.0):
    attrs = f'cx="{cx}" cy="{cy}" r="{r}" fill="{fill}"'
    if opacity < 1.0:
        attrs += f' opacity="{opacity}"'
    return f'  <circle {attrs}/>\n'


def svg_ellipse(cx, cy, rx, ry, fill, opacity=1.0):
    attrs = f'cx="{cx}" cy="{cy}" rx="{rx}" ry="{ry}" fill="{fill}"'
    if opacity < 1.0:
        attrs += f' opacity="{opacity}"'
    return f'  <ellipse {attrs}/>\n'


def svg_polygon(points, fill, opacity=1.0):
    pts = " ".join(f"{x},{y}" for x, y in points)
    attrs = f'points="{pts}" fill="{fill}"'
    if opacity < 1.0:
        attrs += f' opacity="{opacity}"'
    return f'  <polygon {attrs}/>\n'


def svg_path(d, fill="none", stroke="none", stroke_width=1, opacity=1.0):
    attrs = f'd="{d}" fill="{fill}" stroke="{stroke}" stroke-width="{stroke_width}"'
    if opacity < 1.0:
        attrs += f' opacity="{opacity}"'
    return f'  <path {attrs}/>\n'


def svg_text(x, y, text, size=16, fill="white", anchor="middle", font="sans-serif"):
    return (
        f'  <text x="{x}" y="{y}" font-size="{size}" fill="{fill}" '
        f'text-anchor="{anchor}" font-family="{font}" dominant-baseline="central">'
        f'{text}</text>\n'
    )


def svg_group(content, transform=""):
    t = f' transform="{transform}"' if transform else ""
    return f'  <g{t}>\n{content}  </g>\n'


# ---------------------------------------------------------------------------
# Sprite definitions
# ---------------------------------------------------------------------------

def make_coin():
    """Gold coin with a star highlight."""
    s = 64
    content = ""
    # Coin body
    content += svg_circle(32, 32, 28, "#FFD700")
    # Inner ring
    content += svg_circle(32, 32, 22, "none").replace(
        'fill="none"', 'fill="none" stroke="#DAA520" stroke-width="2"'
    )
    # Dollar sign
    content += svg_text(32, 33, "$", size=24, fill="#B8860B")
    # Highlight
    content += svg_circle(22, 20, 6, "white", opacity=0.4)
    return svg_doc(s, s, content)


def make_heart():
    """Red heart — health/life icon."""
    s = 64
    content = ""
    # Heart shape via path
    content += svg_path(
        "M32 56 C32 56 8 40 8 24 C8 14 16 8 24 8 C28 8 32 12 32 16 "
        "C32 12 36 8 40 8 C48 8 56 14 56 24 C56 40 32 56 32 56 Z",
        fill="#E63946"
    )
    # Highlight
    content += svg_circle(24, 22, 5, "white", opacity=0.3)
    return svg_doc(s, s, content)


def make_star():
    """Yellow star — score/collectible icon."""
    s = 64
    import math
    points = []
    for i in range(10):
        angle = math.pi / 2 + i * math.pi / 5
        r = 28 if i % 2 == 0 else 12
        points.append((32 + r * math.cos(angle), 32 - r * math.sin(angle)))
    content = ""
    content += svg_polygon(points, "#FFD700")
    content += svg_polygon(points, "white").replace(
        'fill="white"', 'fill="white" opacity="0.2" transform="translate(-2,-2) scale(0.85)" transform-origin="32 32"'
    )
    return svg_doc(s, s, content)


def make_sword():
    """Simple sword icon — attack/weapon."""
    s = 64
    content = ""
    # Blade
    content += svg_path(
        "M30 8 L34 8 L35 40 L29 40 Z",
        fill="#C0C0C0"
    )
    # Blade edge highlight
    content += svg_path(
        "M30 8 L32 8 L32 40 L30 40 Z",
        fill="#E8E8E8"
    )
    # Guard
    content += svg_rect(22, 40, 20, 5, "#8B4513", rx=2)
    # Grip
    content += svg_rect(29, 45, 6, 12, "#654321", rx=1)
    # Pommel
    content += svg_circle(32, 59, 4, "#FFD700")
    return svg_doc(s, s, content)


def make_shield():
    """Blue shield icon — defense."""
    s = 64
    content = ""
    # Shield body
    content += svg_path(
        "M32 4 C16 4 8 12 8 24 C8 44 32 60 32 60 C32 60 56 44 56 24 C56 12 48 4 32 4 Z",
        fill="#4169E1"
    )
    # Border
    content += svg_path(
        "M32 4 C16 4 8 12 8 24 C8 44 32 60 32 60 C32 60 56 44 56 24 C56 12 48 4 32 4 Z",
        fill="none", stroke="#1E3A8A", stroke_width=3
    )
    # Cross emblem
    content += svg_rect(28, 16, 8, 28, "#FFD700", rx=1)
    content += svg_rect(18, 24, 28, 8, "#FFD700", rx=1)
    return svg_doc(s, s, content)


def make_potion():
    """Red potion bottle — health pickup."""
    s = 64
    content = ""
    # Bottle neck
    content += svg_rect(27, 10, 10, 12, "#D4D4D4", rx=2)
    # Cork
    content += svg_rect(28, 6, 8, 6, "#8B4513", rx=2)
    # Bottle body
    content += svg_path(
        "M22 22 L22 22 Q20 26 20 32 L20 50 Q20 56 26 56 L38 56 Q44 56 44 50 L44 32 Q44 26 42 22 Z",
        fill="#E63946"
    )
    # Liquid shine
    content += svg_ellipse(30, 40, 6, 10, "white", opacity=0.2)
    # Glass highlight
    content += svg_path(
        "M25 28 Q24 34 25 46",
        fill="none", stroke="white", stroke_width=2, opacity=0.4
    )
    return svg_doc(s, s, content)


SPRITES = {
    "Coin": make_coin,
    "Heart": make_heart,
    "Star": make_star,
    "Sword": make_sword,
    "Shield": make_shield,
    "Potion": make_potion,
}


# ---------------------------------------------------------------------------
# Export
# ---------------------------------------------------------------------------

def export_png(svg_path, png_path, size):
    """Convert SVG to PNG via Inkscape CLI."""
    cmd = [
        INKSCAPE,
        svg_path,
        f"--export-filename={png_path}",
        f"--export-width={size}",
        f"--export-height={size}",
    ]
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        print(f"  Inkscape error: {result.stderr.strip()}")
        return False
    return True


def main():
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    flags = [a for a in sys.argv[1:] if a.startswith("--")]

    output_dir = args[0] if args else "."
    sprite_filter = args[1].split(",") if len(args) > 1 else None

    size = DEFAULT_SIZE
    for flag in flags:
        if flag.startswith("--size="):
            size = int(flag.split("=")[1])

    os.makedirs(output_dir, exist_ok=True)

    for name, generator in SPRITES.items():
        if sprite_filter and name not in sprite_filter:
            continue

        svg_content = generator()
        svg_file = os.path.join(output_dir, f"SPR_{name}.svg")
        png_file = os.path.join(output_dir, f"SPR_{name}.png")

        with open(svg_file, "w", encoding="utf-8") as f:
            f.write(svg_content)
        print(f"  SVG: {svg_file}")

        if export_png(svg_file, png_file, size):
            print(f"  PNG: {png_file} ({size}x{size})")
        else:
            print(f"  PNG export failed for {name}")

    print("Done.")


if __name__ == "__main__":
    main()
