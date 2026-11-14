#!/usr/bin/env python3
# Generate Plasma Style SVGs, color schemes, and Konsole schemes for Material Dark.
# SPDX-License-Identifier: MIT
"""Regenerate the Material Dark assets from one palette per variant.

Usage:
    python3 tools/gen-plasma-assets.py            # both variants
    python3 tools/gen-plasma-assets.py pink       # only pink
    python3 tools/gen-plasma-assets.py blue       # only blue
"""
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
VERSION = "1.3.1"

# ---------------------------------------------------------------------------
# Palettes: pink (default) and blue. Satu sumber untuk Plasma SVG, color
# scheme, dan Konsole. Semua radius tetap di sini sudah dipotong 50%.
# ---------------------------------------------------------------------------
PALETTES = {
    "pink": {
        "id": "material-dark-kde",
        "name": "Material Dark",
        "scheme": "MaterialDark",
        "desc": "Material Design 3 dark: warm charcoal surfaces with soft pink accents, rounded shapes",
        "primary": "#F2B8C6",
        "on_primary": "#2B1219",
        "primary_container": "#FFD9E2",
        "view": "#100E11",
        "view_alt": "#211C1F",
        "window": "#1A1619",
        "section": "#272225",
        "button": "#322A2E",
        "button_alt": "#3A3034",
        "track": "#3A3538",
        "field": "#1D1B1E",
        "text": "#F0E4E7",
        "inactive": "#CDB9BF",
        "hairline": "#453F42",
        "tooltip": "#322A2E",
        "titlebar": "#2C2528",
        "header_inactive": "#181417",
        "wm_inactive": "#A09096",
        "disabled": "#463C40",
        "inactive_effect": "#968288",
        "error": "#F2B8B5",
        "warning": "#F5D3A8",
        "positive": "#A8DAB5",
        "link": "#FFD9E2",
        "visited": "#D8B4C0",
        "sel_fg_inactive": "#4A1F2A",
        "sel_link": "#004A77",
        "sel_neg": "#93000A",
        "sel_neutral": "#6E3C00",
        "sel_pos": "#005023",
        "sel_visited": "#652564",
        "konsole_bg": "#141013",
        "konsole_faint": "#1A1619",
        "konsole_intense": "#100E11",
        "ansi": [
            ("#3A3034", "#2B2528", "#A09096"),
            ("#F2B8B5", "#BE9290", "#FFDAD6"),
            ("#A8DAB5", "#84AC8F", "#C8F0D0"),
            ("#F5D3A8", "#C4A886", "#FFE8C8"),
            ("#A8C7FA", "#849CC4", "#D3E3FD"),
            ("#F2B8C6", "#C2939E", "#FFD9E2"),
            ("#80D5E0", "#64A7B0", "#B2EBF2"),
            ("#F0E4E7", "#CDB9BF", "#FFFFFF"),
        ],
    },
    "blue": {
        "id": "material-dark-kde-blue",
        "name": "Material Dark Blue",
        "scheme": "MaterialDarkBlue",
        "desc": "Material Design 3 dark: cool charcoal surfaces with soft blue accents, rounded shapes",
        "primary": "#A8C7FA",
        "on_primary": "#0A305F",
        "primary_container": "#D3E3FD",
        "view": "#0D1117",
        "view_alt": "#1C2026",
        "window": "#151A21",
        "section": "#21262E",
        "button": "#2B323C",
        "button_alt": "#343C48",
        "track": "#343940",
        "field": "#1D2229",
        "text": "#E3E8F0",
        "inactive": "#B6C0CE",
        "hairline": "#3E434A",
        "tooltip": "#2B323C",
        "titlebar": "#252A33",
        "header_inactive": "#13181F",
        "wm_inactive": "#8C96A8",
        "disabled": "#464E5A",
        "inactive_effect": "#8892A2",
        "error": "#F2B8B5",
        "warning": "#F5D3A8",
        "positive": "#A8DAB5",
        "link": "#D3E3FD",
        "visited": "#C9B8E8",
        "sel_fg_inactive": "#1D3A5F",
        "sel_link": "#00325B",
        "sel_neg": "#690005",
        "sel_neutral": "#402D00",
        "sel_pos": "#00391C",
        "sel_visited": "#3F2A55",
        "konsole_bg": "#10141A",
        "konsole_faint": "#151A21",
        "konsole_intense": "#0D1117",
        "ansi": [
            ("#2B323C", "#222830", "#8C96A8"),
            ("#F2B8B5", "#BE9290", "#FFDAD6"),
            ("#A8DAB5", "#84AC8F", "#C8F0D0"),
            ("#F5D3A8", "#C4A886", "#FFE8C8"),
            ("#A8C7FA", "#849CC4", "#D3E3FD"),
            ("#C9B8E8", "#9E90B6", "#E0D3F8"),
            ("#80D5E0", "#64A7B0", "#B2EBF2"),
            ("#E3E8F0", "#B6C0CE", "#FFFFFF"),
        ],
    },
}


def csv(hex_color):
    h = hex_color.lstrip("#")
    return f"{int(h[0:2], 16)},{int(h[2:4], 16)},{int(h[4:6], 16)}"


def rect(x, y, w, h, color, opacity=None):
    extra = f' fill-opacity="{opacity}"' if opacity is not None else ""
    return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{color}"{extra}/>'


def rr_parts(prefix, x, y, w, h, r, color):
    """9-slice pieces of a filled rounded rectangle."""
    p = (prefix + "-") if prefix else ""
    return [
        f'<g id="{p}topleft" transform="translate({x},{y})"><path d="M0,{r} A{r},{r} 0 0 1 {r},0 L{r},{r} Z" fill="{color}"/></g>',
        f'<g id="{p}topright" transform="translate({x + w - r},{y})"><path d="M0,0 A{r},{r} 0 0 1 {r},{r} L0,{r} Z" fill="{color}"/></g>',
        f'<g id="{p}bottomleft" transform="translate({x},{y + h - r})"><path d="M0,0 A{r},{r} 0 0 0 {r},{r} L{r},0 Z" fill="{color}"/></g>',
        f'<g id="{p}bottomright" transform="translate({x + w - r},{y + h - r})"><path d="M{r},0 A{r},{r} 0 0 1 0,{r} L0,0 Z" fill="{color}"/></g>',
        f'<g id="{p}top" transform="translate({x + r},{y})">' + rect(0, 0, w - 2 * r, r, color) + "</g>",
        f'<g id="{p}bottom" transform="translate({x + r},{y + h - r})">' + rect(0, 0, w - 2 * r, r, color) + "</g>",
        f'<g id="{p}left" transform="translate({x},{y + r})">' + rect(0, 0, r, h - 2 * r, color) + "</g>",
        f'<g id="{p}right" transform="translate({x + w - r},{y + r})">' + rect(0, 0, r, h - 2 * r, color) + "</g>",
        f'<g id="{p}center" transform="translate({x + r},{y + r})">' + rect(0, 0, w - 2 * r, h - 2 * r, color) + "</g>",
    ]


def frame(prefix, x, y, w, h, r, fill, border=None, bw=2, pad=0):
    """9-slice rounded rectangle, optionally with a real border ring.

    Every piece contains both the border and the fill: an overlay frame with
    different element names would be ignored by KSvg.

    ``pad`` pushes the painted art ``pad`` pixels inwards while keeping the
    slice geometry larger. TextField overlays are anchored with negative
    margins, so a padded overlay ring can land exactly on the base frame edge
    instead of drawing a second ring outside it.
    """
    p = (prefix + "-") if prefix else ""

    if not border:
        return rr_parts(prefix, x, y, w, h, r, fill)

    ir = max(1, r - bw)
    tl_outer = f"M0,{r} A{r},{r} 0 0 1 {r},0 L{r},{r} Z"
    tl_inner = f"M{bw},{r} A{ir},{ir} 0 0 1 {r},{bw} L{r},{r} Z"
    tr_outer = f"M0,0 A{r},{r} 0 0 1 {r},{r} L0,{r} Z"
    tr_inner = f"M0,{bw} A{ir},{ir} 0 0 1 {r - bw},{r} L0,{r} Z"
    bl_outer = f"M0,0 A{r},{r} 0 0 0 {r},{r} L{r},0 Z"
    bl_inner = f"M{bw},0 A{ir},{ir} 0 0 0 {r},{r - bw} L{r},0 Z"
    br_outer = f"M{r},0 A{r},{r} 0 0 1 0,{r} L0,0 Z"
    br_inner = f"M{r - bw},0 A{ir},{ir} 0 0 1 0,{r - bw} L0,0 Z"

    def wrap(name, gx, gy, bw_box, bh_box, art, ax, ay):
        if not pad:
            return f'<g id="{name}" transform="translate({gx},{gy})">{art}</g>'
        return (
            f'<g id="{name}" transform="translate({gx},{gy})">'
            + rect(0, 0, bw_box, bh_box, "#000", "0")
            + f'<g transform="translate({ax},{ay})">{art}</g>'
            + "</g>"
        )

    def corner(name, px, py, outer, inner, right=False, bottom=False):
        art = f'<path d="{outer}" fill="{border}"/><path d="{inner}" fill="{fill}"/>'
        if not pad:
            return f'<g id="{name}" transform="translate({px},{py})">{art}</g>'
        ox = -pad
        oy = -pad
        art_off = (pad, pad)
        if right:
            ox = 0
        if bottom:
            oy = 0
        if right and bottom:
            art_off = (0, 0)
        elif right:
            art_off = (0, pad)
        elif bottom:
            art_off = (pad, 0)
        else:
            ox, oy, art_off = -pad, -pad, (pad, pad)
        return (
            f'<g id="{name}" transform="translate({px + ox},{py + oy})">'
            + rect(0, 0, r + pad, r + pad, "#000", "0")
            + f'<g transform="translate({art_off[0]},{art_off[1]})">{art}</g>'
            + "</g>"
        )

    top_box = (w - 2 * r, r + pad) if pad else (w - 2 * r, r)
    left_box = (r + pad, h - 2 * r) if pad else (r, h - 2 * r)

    return [
        corner(p + "topleft", x, y, tl_outer, tl_inner),
        corner(p + "topright", x + w - r, y, tr_outer, tr_inner, right=True),
        corner(p + "bottomleft", x, y + h - r, bl_outer, bl_inner, bottom=True),
        corner(p + "bottomright", x + w - r, y + h - r, br_outer, br_inner, right=True, bottom=True),
        wrap(
            p + "top", x + r, y - pad, top_box[0], top_box[1],
            rect(0, 0, w - 2 * r, bw, border) + rect(0, bw, w - 2 * r, r - bw, fill),
            0, pad,
        ),
        wrap(
            p + "bottom", x + r, y + h - r, top_box[0], top_box[1],
            rect(0, 0, w - 2 * r, r - bw, fill) + rect(0, r - bw, w - 2 * r, bw, border),
            0, 0,
        ),
        wrap(
            p + "left", x - pad, y + r, left_box[0], left_box[1],
            rect(0, 0, bw, h - 2 * r, border) + rect(bw, 0, r - bw, h - 2 * r, fill),
            pad, 0,
        ),
        wrap(
            p + "right", x + w - r, y + r, left_box[0], left_box[1],
            rect(0, 0, r - bw, h - 2 * r, fill) + rect(r - bw, 0, bw, h - 2 * r, border),
            0, 0,
        ),
        f'<g id="{p}center" transform="translate({x + r},{y + r})">' + rect(0, 0, w - 2 * r, h - 2 * r, fill) + "</g>",
    ]


def frame_hints(prefix, radius, x, y, natural):
    """Transparent elements whose *size* becomes the frame content margin."""
    base = (prefix + "-") if prefix else ""
    return [
        f'<g id="{base}hint-left-margin">' + rect(x, y + 20, radius, 4, "#000", "0") + "</g>",
        f'<g id="{base}hint-right-margin">' + rect(x + natural - radius, y + 20, radius, 4, "#000", "0") + "</g>",
        f'<g id="{base}hint-top-margin">' + rect(x + 20, y, 4, radius, "#000", "0") + "</g>",
        f'<g id="{base}hint-bottom-margin">' + rect(x + 20, y + natural - radius, 4, radius, "#000", "0") + "</g>",
    ]


def shadow_defs(pad, r):
    """Gradien bayangan lembut untuk frame shell (Material elevation)."""
    R = r + pad
    return (
        "<defs>\n"
        f'<linearGradient id="md-sh-top" gradientUnits="userSpaceOnUse" x1="0" y1="0" x2="0" y2="{pad}">'
        '<stop offset="0" stop-color="#000" stop-opacity="0.02"/>'
        '<stop offset="0.6" stop-color="#000" stop-opacity="0.16"/>'
        '<stop offset="1" stop-color="#000" stop-opacity="0.34"/>'
        "</linearGradient>\n"
        f'<linearGradient id="md-sh-bottom" gradientUnits="userSpaceOnUse" x1="0" y1="{r}" x2="0" y2="{r + pad}">'
        '<stop offset="0" stop-color="#000" stop-opacity="0.34"/>'
        '<stop offset="0.4" stop-color="#000" stop-opacity="0.16"/>'
        '<stop offset="1" stop-color="#000" stop-opacity="0.02"/>'
        "</linearGradient>\n"
        f'<linearGradient id="md-sh-left" gradientUnits="userSpaceOnUse" x1="0" y1="0" x2="{pad}" y2="0">'
        '<stop offset="0" stop-color="#000" stop-opacity="0.02"/>'
        '<stop offset="0.6" stop-color="#000" stop-opacity="0.16"/>'
        '<stop offset="1" stop-color="#000" stop-opacity="0.34"/>'
        "</linearGradient>\n"
        f'<linearGradient id="md-sh-right" gradientUnits="userSpaceOnUse" x1="{r}" y1="0" x2="{r + pad}" y2="0">'
        '<stop offset="0" stop-color="#000" stop-opacity="0.34"/>'
        '<stop offset="0.4" stop-color="#000" stop-opacity="0.16"/>'
        '<stop offset="1" stop-color="#000" stop-opacity="0.02"/>'
        "</linearGradient>\n"
        + "".join(
            f'<radialGradient id="md-sh-{name}" gradientUnits="userSpaceOnUse" cx="{cx}" cy="{cy}" r="{R}">'
            '<stop offset="0" stop-color="#000" stop-opacity="0.34"/>'
            '<stop offset="0.55" stop-color="#000" stop-opacity="0.14"/>'
            '<stop offset="1" stop-color="#000" stop-opacity="0"/>'
            "</radialGradient>\n"
            for name, cx, cy in (
                ("tl", pad, pad),
                ("tr", r, pad),
                ("bl", pad, r),
                ("br", r, r),
            )
        )
        + "</defs>"
    )


def frame_shadow(prefix, x, y, w, h, r, fill, border, bw, pad):
    """9-slice rounded frame dengan bayangan di padding luar body.

    Body tetap berukuran ``w x h``; padding ``pad`` ikut masuk ke dalam
    potongan 9-slice sehingga Plasma menggambar bayangan di dalam window
    (isi popup tidak bergeser karena hint margin ikut bertambah).
    """
    p = (prefix + "-") if prefix else ""
    R = r + pad
    ir = max(1, r - bw)

    def body_corner(kind):
        if kind == "tl":
            outer = f"M0,{r} A{r},{r} 0 0 1 {r},0 L{r},{r} Z"
            inner = f"M{bw},{r} A{ir},{ir} 0 0 1 {r},{bw} L{r},{r} Z"
            offset = (pad, pad)
        elif kind == "tr":
            outer = f"M0,0 A{r},{r} 0 0 1 {r},{r} L0,{r} Z"
            inner = f"M0,{bw} A{ir},{ir} 0 0 1 {r - bw},{r} L0,{r} Z"
            offset = (0, pad)
        elif kind == "bl":
            outer = f"M0,0 A{r},{r} 0 0 0 {r},{r} L{r},0 Z"
            inner = f"M{bw},0 A{ir},{ir} 0 0 0 {r},{r - bw} L{r},0 Z"
            offset = (pad, 0)
        else:
            outer = f"M{r},0 A{r},{r} 0 0 1 0,{r} L0,0 Z"
            inner = f"M{r - bw},0 A{ir},{ir} 0 0 1 0,{r - bw} L0,0 Z"
            offset = (0, 0)
        art = f'<path d="{outer}" fill="{border}"/><path d="{inner}" fill="{fill}"/>'
        return offset, art

    def corner(kind, gx, gy, shadow_path):
        offset, art = body_corner(kind)
        return (
            f'<g id="{p}{kind}" transform="translate({gx},{gy})">'
            f'<path d="{shadow_path}" fill="url(#md-sh-{kind})"/>'
            f'<g transform="translate({offset[0]},{offset[1]})">{art}</g>'
            "</g>"
        )

    # corner shadow paths memakai radius luar R
    tl_path = f"M0,{R} A{R},{R} 0 0 1 {R},0 L{R},{R} Z"
    tr_path = f"M0,0 A{R},{R} 0 0 1 {R},{R} L0,{R} Z"
    bl_path = f"M0,0 A{R},{R} 0 0 0 {R},{R} L{R},0 Z"
    br_path = f"M{R},0 A{R},{R} 0 0 1 0,{R} L0,0 Z"

    return [
        corner("tl", x - pad, y - pad, tl_path),
        corner("tr", x + w - r, y - pad, tr_path),
        corner("bl", x - pad, y + h - r, bl_path),
        corner("br", x + w - r, y + h - r, br_path),
        f'<g id="{p}top" transform="translate({x + r},{y - pad})">'
        + rect(0, 0, w - 2 * r, pad, "url(#md-sh-top)")
        + rect(0, pad, w - 2 * r, bw, border)
        + rect(0, pad + bw, w - 2 * r, r - bw, fill)
        + "</g>",
        f'<g id="{p}bottom" transform="translate({x + r},{y + h - r})">'
        + rect(0, 0, w - 2 * r, r - bw, fill)
        + rect(0, r - bw, w - 2 * r, bw, border)
        + rect(0, r, w - 2 * r, pad, "url(#md-sh-bottom)")
        + "</g>",
        f'<g id="{p}left" transform="translate({x - pad},{y + r})">'
        + rect(0, 0, pad, h - 2 * r, "url(#md-sh-left)")
        + rect(pad, 0, bw, h - 2 * r, border)
        + rect(pad + bw, 0, r - bw, h - 2 * r, fill)
        + "</g>",
        f'<g id="{p}right" transform="translate({x + w - r},{y + r})">'
        + rect(0, 0, r - bw, h - 2 * r, fill)
        + rect(r - bw, 0, bw, h - 2 * r, border)
        + rect(r, 0, pad, h - 2 * r, "url(#md-sh-right)")
        + "</g>",
        f'<g id="{p}center" transform="translate({x + r},{y + r})">'
        + rect(0, 0, w - 2 * r, h - 2 * r, fill)
        + "</g>",
    ]


def frame_shadow_hints(prefix, radius, x, y, natural):
    """Hint margin = padding bayangan + radius supaya isi tetap di body."""
    base = (prefix + "-") if prefix else ""
    return [
        f'<g id="{base}hint-left-margin">' + rect(x, y + 20, radius, 4, "#000", "0") + "</g>",
        f'<g id="{base}hint-right-margin">' + rect(x + natural - radius, y + 20, radius, 4, "#000", "0") + "</g>",
        f'<g id="{base}hint-top-margin">' + rect(x + 20, y, 4, radius, "#000", "0") + "</g>",
        f'<g id="{base}hint-bottom-margin">' + rect(x + 20, y + natural - radius, 4, radius, "#000", "0") + "</g>",
    ]


def write_svg(path, width, height, body):
    path.parent.mkdir(parents=True, exist_ok=True)
    text = '<?xml version="1.0" encoding="UTF-8"?>\n'
    text += f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}">\n'
    text += "\n".join(body)
    text += "\n</svg>\n"
    path.write_text(text)
    print("wrote", path.relative_to(ROOT))


def simple_frame(path, radius, fill, hairline, natural=64, pad=10):
    # Plasma 6.8 looks up dialog/popup frames with an empty prefix, while a few
    # components still request "normal". Emit both sets so the surface is never
    # silently dropped (that made app-menu popups fully transparent).
    natural = max(natural, 2 * radius + 8)
    body = [shadow_defs(pad, radius)]
    for prefix in ("", "normal"):
        body += frame_shadow(prefix, pad, pad, natural, natural, radius, fill, hairline, 1, pad)
        body += frame_shadow_hints(prefix, radius + pad, pad, pad, natural)
    write_svg(path, natural + 2 * pad, natural + 2 * pad, body)


def gen_backgrounds(p, widgets, dialogs):
    # Setengah dari radius sebelumnya: 16 -> 8, 20 -> 10, 10 -> 5.
    simple_frame(widgets / "background.svg", 8, p["section"], p["hairline"], pad=10)
    simple_frame(dialogs / "background.svg", 10, p["section"], p["hairline"], pad=10)
    simple_frame(widgets / "tooltip.svg", 5, p["tooltip"], p["hairline"], pad=8)


def gen_viewitem(p, widgets):
    w, h, r = 48, 40, 4
    states = [
        ("normal", 10, None, "0"),
        ("hover", 70, p["text"], "0.08"),
        ("selected", 130, p["primary"], None),
        ("selected+hover", 190, p["primary_container"], None),
    ]
    body = []
    for name, y, color, opacity in states:
        if color is None:
            body += [
                f'<g id="{name}-topleft"><path transform="translate(10,{y})" d="M0,{r} A{r},{r} 0 0 1 {r},0 L{r},{r} Z" fill="#000" fill-opacity="0"/></g>',
                f'<g id="{name}-topright"><path transform="translate({10 + w - r},{y})" d="M0,0 A{r},{r} 0 0 1 {r},{r} L0,{r} Z" fill="#000" fill-opacity="0"/></g>',
                f'<g id="{name}-bottomleft"><path transform="translate(10,{y + h - r})" d="M0,0 A{r},{r} 0 0 0 {r},{r} L{r},0 Z" fill="#000" fill-opacity="0"/></g>',
                f'<g id="{name}-bottomright"><path transform="translate({10 + w - r},{y + h - r})" d="M{r},0 A{r},{r} 0 0 1 0,{r} L0,0 Z" fill="#000" fill-opacity="0"/></g>',
                f'<g id="{name}-top">' + rect(10 + r, y, w - 2 * r, r, "#000", "0") + "</g>",
                f'<g id="{name}-bottom">' + rect(10 + r, y + h - r, w - 2 * r, r, "#000", "0") + "</g>",
                f'<g id="{name}-left">' + rect(10, y + r, r, h - 2 * r, "#000", "0") + "</g>",
                f'<g id="{name}-right">' + rect(10 + w - r, y + r, r, h - 2 * r, "#000", "0") + "</g>",
                f'<g id="{name}-center">' + rect(10 + r, y + r, w - 2 * r, h - 2 * r, "#000", "0") + "</g>",
            ]
        else:
            body += rr_parts(name, 10, y, w, h, r, color)
            if opacity:
                for i in range(len(body) - 9, len(body)):
                    body[i] = body[i].replace('fill="' + color + '"', f'fill="{color}" fill-opacity="{opacity}"')
    body.append('<g id="hint-tile-center">' + rect(10, 260, 8, 8, "#000", "0") + "</g>")
    write_svg(widgets / "viewitem.svg", 90, 300, body)


def pill3(prefix, x, y, width, height, color):
    r = height / 2
    r = int(r) if r == int(r) else r
    return [
        f'<g id="{prefix}-left" transform="translate({x},{y})"><path d="M{r},0 A{r},{r} 0 0 0 {r},{height} L{r},0 Z" fill="{color}"/></g>',
        f'<g id="{prefix}-center" transform="translate({x + r},{y})">' + rect(0, 0, width, height, color) + "</g>",
        f'<g id="{prefix}-right" transform="translate({x + r + width},{y})"><path d="M0,0 A{r},{r} 0 0 1 0,{height} L0,0 Z" fill="{color}"/></g>',
    ]


def gen_bar(path, p, vertical=False):
    height = 12
    body = []
    body += pill3("bar-inactive", 10, 10, 28, height, p["track"])
    body += pill3("bar-active", 10, 40, 28, height, p["primary"])
    if vertical:
        # vertical meter: same assets but 12px wide bars
        pass
    body.append('<g id="hint-bar-size">' + rect(10, 70, 40, height, "#000", "0") + "</g>")
    body.append('<g id="hint-tile-center">' + rect(10, 90, 4, 4, "#000", "0") + "</g>")
    body.append('<g id="hint-stretch-borders">' + rect(10, 96, 4, 4, "#000", "0") + "</g>")
    write_svg(path, 90, 120, body)


def gen_switch(p, widgets):
    h = 22
    body = []
    body += pill3("inactive", 10, 10, 22, h, p["track"])
    body += pill3("active", 10, 42, 22, h, p["primary"])

    # Handle: 24x24 cell, 19px dark knob with an accent ring so it reads on
    # both the dark and the accent track.
    def handle(name, y, fill, ring, ring_opacity=1.0):
        return (
            f'<g id="{name}" transform="translate(10,{y})">'
            + rect(0, 0, 24, 24, "#000", "0")
            + f'<circle cx="12" cy="12" r="8.5" fill="{fill}" stroke="{ring}" stroke-width="2" stroke-opacity="{ring_opacity}"/>'
            + "</g>"
        )

    body.append(handle("handle", 80, p["on_primary"], p["primary"]))
    body.append(handle("handle-active", 110, p["on_primary"], p["primary"]))
    body.append(handle("handle-hover", 140, p["on_primary"], p["primary_container"]))
    body.append(handle("handle-pressed", 170, p["on_primary"], p["primary_container"]))
    body.append(
        '<g id="handle-focus" transform="translate(10,200)">'
        + rect(0, 0, 32, 32, "#000", "0")
        + f'<circle cx="16" cy="16" r="13" fill="none" stroke="{p["primary"]}" stroke-width="2" stroke-opacity="0.7"/>'
        + "</g>"
    )
    body.append('<g id="handle-shadow" transform="translate(10,240)">' + rect(0, 0, 24, 24, "#000", "0") + "</g>")
    body.append('<g id="hint-bar-size">' + rect(10, 270, 44, h, "#000", "0") + "</g>")
    body.append('<g id="hint-stretch-borders">' + rect(10, 296, 4, 4, "#000", "0") + "</g>")
    write_svg(widgets / "switch.svg", 90, 320, body)


def gen_lineedit(p, widgets):
    # Base setengah dari 8 -> 4. Base memakai permukaan view, hover/fokus
    # memakai field tonal; fokus tetap satu cincin primary 2px.
    w, h, r, bw = 64, 36, 4, 2
    rows = [
        ("base", 10, p["view"], None),
        ("hover", 70, p["field"], p["field"]),
        ("focus", 130, p["field"], p["primary"]),
        ("focusframe", 190, p["field"], p["primary"]),
    ]
    body = []
    # Draw the focus overlay above the base (PlasmaComponents TextField checks
    # this id and otherwise puts the focus layer behind the background).
    body.append('<g id="hint-focus-over-base">' + rect(10, 10, 4, 4, "#000", "0") + "</g>")
    body += frame("base", 10, 10, w, h, r, p["view"])
    for name, y, fill, border in rows[1:]:
        body += frame(name, 10, y, w, h, r, fill, border, bw, pad=r)
        body += [
            f'<g id="{name}-hint-left-margin">' + rect(10, y + 20, r, 4, "#000", "0") + "</g>",
            f'<g id="{name}-hint-right-margin">' + rect(10 + w - r, y + 20, r, 4, "#000", "0") + "</g>",
            f'<g id="{name}-hint-top-margin">' + rect(10 + 20, y, 4, r, "#000", "0") + "</g>",
            f'<g id="{name}-hint-bottom-margin">' + rect(10 + 20, y + h - r, 4, r, "#000", "0") + "</g>",
        ]
    write_svg(widgets / "lineedit.svg", 100, 250, body)


def gen_slider(p, widgets):
    body = []
    body.append('<g id="hint-handle-size">' + rect(10, 10, 20, 20, "#000", "0") + "</g>")
    for direction, y in (("horizontal", 36), ("vertical", 200)):
        body.append(
            f'<g id="{direction}-slider-handle" transform="translate(10,{y})">'
            + rect(0, 0, 20, 20, "#000", "0")
            + f'<circle cx="10" cy="10" r="8" fill="{p["on_primary"]}" stroke="{p["primary"]}" stroke-width="2"/>'
            + "</g>"
        )
        body.append(f'<g id="{direction}-slider-shadow" transform="translate(10,{y + 24})">' + rect(0, 0, 20, 20, "#000", "0") + "</g>")
        body.append(
            f'<g id="{direction}-slider-hover" transform="translate(10,{y + 48})">'
            + rect(0, 0, 36, 36, "#000", "0")
            + f'<circle cx="18" cy="18" r="17" fill="none" stroke="{p["primary"]}" stroke-width="2" stroke-opacity="0.45"/>'
            + "</g>"
        )
        body.append(
            f'<g id="{direction}-slider-focus" transform="translate(10,{y + 88})">'
            + rect(0, 0, 36, 36, "#000", "0")
            + f'<circle cx="18" cy="18" r="17" fill="none" stroke="{p["primary"]}" stroke-width="2" stroke-opacity="0.75"/>'
            + "</g>"
        )
    # Groove: 12px tall rounded track (4px caps), same structure as the
    # original theme so Slider.qml's fixedMargins add up to 12.
    r = 4
    for prefix, y, color in (("groove", 340, p["track"]), ("groove-highlight", 370, p["primary"])):
        body += [
            f'<g id="{prefix}-topleft" transform="translate(10,{y})"><path d="M0,{r} A{r},{r} 0 0 1 {r},0 L{r},{r} Z" fill="{color}"/></g>',
            f'<g id="{prefix}-topright" transform="translate({10 + 4 + 16},{y})"><path d="M0,0 A{r},{r} 0 0 1 {r},{r} L0,{r} Z" fill="{color}"/></g>',
            f'<g id="{prefix}-bottomleft" transform="translate(10,{y + 4 + 4})"><path d="M0,0 A{r},{r} 0 0 0 {r},{r} L{r},0 Z" fill="{color}"/></g>',
            f'<g id="{prefix}-bottomright" transform="translate({10 + 4 + 16},{y + 4 + 4})"><path d="M{r},0 A{r},{r} 0 0 1 0,{r} L0,0 Z" fill="{color}"/></g>',
            f'<g id="{prefix}-top">' + rect(10 + 4, y, 16, 4, color) + "</g>",
            f'<g id="{prefix}-bottom">' + rect(10 + 4, y + 8, 16, 4, color) + "</g>",
            f'<g id="{prefix}-left">' + rect(10, y + 4, 4, 4, color) + "</g>",
            f'<g id="{prefix}-right">' + rect(10 + 4 + 16, y + 4, 4, 4, color) + "</g>",
            f'<g id="{prefix}-center">' + rect(10 + 4, y + 4, 16, 4, color) + "</g>",
        ]
    write_svg(widgets / "slider.svg", 90, 400, body)


def gen_panel(p, widgets, solid_widgets):
    # Plasma 6 panel-background is used in three forms:
    #   * unprefixed elements     -> non-floating panel (adaptive/translucent)
    #   * prefix "floating"       -> floating panel (9px pill + 9px padding)
    #   * solid/widgets/...       -> opaque panel when a window touches it
    # The unprefixed frame must be flat edge-to-edge; a rounded unprefixed
    # frame would leave gaps at the screen edges.
    w, h, edge = 64, 48, 4
    x, y = 10, 10
    body = [
        f'<g id="topleft">' + rect(x, y, edge, edge, p["window"]) + "</g>",
        f'<g id="top">' + rect(x + edge, y, w - 2 * edge, edge, p["window"]) + "</g>",
        f'<g id="topright">' + rect(x + w - edge, y, edge, edge, p["window"]) + "</g>",
        f'<g id="left">' + rect(x, y + edge, edge, h - 2 * edge, p["window"]) + "</g>",
        f'<g id="center">' + rect(x + edge, y + edge, w - 2 * edge, h - 2 * edge, p["window"]) + "</g>",
        f'<g id="right">' + rect(x + w - edge, y + edge, edge, h - 2 * edge, p["window"]) + "</g>",
        f'<g id="bottomleft">' + rect(x, y + h - edge, edge, edge, p["window"]) + "</g>",
        f'<g id="bottom">' + rect(x + edge, y + h - edge, w - 2 * edge, edge, p["window"]) + "</g>",
        f'<g id="bottomright">' + rect(x + w - edge, y + h - edge, edge, edge, p["window"]) + "</g>",
    ]
    # Padding hints (transparent); size = fixed margin.
    body += [
        '<g id="hint-left-margin">' + rect(x, y + 20, 4, 4, "#000", "0") + "</g>",
        '<g id="hint-right-margin">' + rect(x + w - 4, y + 20, 4, 4, "#000", "0") + "</g>",
        '<g id="hint-top-margin">' + rect(x + 20, y, 4, 4, "#000", "0") + "</g>",
        '<g id="hint-bottom-margin">' + rect(x + 20, y + h - 4, 4, 4, "#000", "0") + "</g>",
    ]
    # Floating variant: 9px rounded pill, 9px float padding (was 18px).
    # Panel mengambang: bayangan 8px di padding frame floating.
    fpad = 8
    fx, fy, fw, fh, fr = 10 + fpad, 80 + fpad, 64, 48, 9
    body.insert(0, shadow_defs(fpad, fr))
    body += frame_shadow("floating", fx, fy, fw, fh, fr, p["window"], p["hairline"], 1, fpad)
    body += frame_shadow_hints("floating", fr + fpad, fx, fy, fw)
    write_svg(widgets / "panel-background.svg", 110, 190, body)
    # Opaque (solid) variant: Plasma paints this when a window touches the panel.
    write_svg(solid_widgets / "panel-background.svg", 110, 190, body)


# ---------------------------------------------------------------------------
# button.svg: 9-slice button states, radius 7 (half of 14). The element ids
# and transforms are preserved; only the slice geometry and palette change.
# ---------------------------------------------------------------------------
BUTTON_FAMILIES = {
    "normal": ("ColorScheme-ButtonBackground", None),
    "hover": ("ColorScheme-Text", "0.08"),
    "pressed": ("ColorScheme-Highlight", "0.22"),
    "toolbutton-hover": ("ColorScheme-Text", "0.08"),
    "toolbutton-pressed": ("ColorScheme-Text", "0.10"),
}


def _btn_art(family, gid, part):
    fill_cls, opacity = BUTTON_FAMILIES.get(family, ("ColorScheme-Highlight", None))
    ring = family in ("focus", "toolbutton-focus")
    o = 9 if ring else 7          # outer radius
    i = 7 if ring else 7          # inner radius (ring thickness 2)
    box = o

    def style(cls):
        extra = f";fill-opacity:{opacity}" if opacity else ""
        return f'class="{cls}" style="fill:currentColor{extra}"'

    if part == "topleft":
        d = (f"M0,{o} A{o},{o} 0 0 1 {o},0 L{o},{i} A{i},{i} 0 0 0 {i},{o} Z"
             if ring else f"M0,{o} A{o},{o} 0 0 1 {o},0 L{o},{o} Z")
        art = f'<rect width="{box}" height="{box}" fill="#000" fill-opacity="0"/>'
        if ring:
            art += f'<path d="{d}" {style(fill_cls)}/>'
        else:
            art += f'<path d="{d}" {style(fill_cls)}/>'
        return art
    if part == "topright":
        d = (f"M0,0 A{o},{o} 0 0 1 {o},{o} L{i},{o} A{i},{i} 0 0 0 0,{i} Z"
             if ring else f"M0,0 A{o},{o} 0 0 1 {o},{o} L0,{o} Z")
        return f'<rect width="{box}" height="{box}" fill="#000" fill-opacity="0"/><path d="{d}" {style(fill_cls)}/>'
    if part == "bottomleft":
        d = (f"M0,0 A{o},{o} 0 0 0 {o},{o} L{o},{i} A{i},{i} 0 0 1 {i},0 Z"
             if ring else f"M0,0 A{o},{o} 0 0 0 {o},{o} L{o},0 Z")
        return f'<rect width="{box}" height="{box}" fill="#000" fill-opacity="0"/><path d="{d}" {style(fill_cls)}/>'
    if part == "bottomright":
        d = (f"M{o},0 A{o},{o} 0 0 1 0,{o} L0,{i} A{i},{i} 0 0 0 {i},0 Z"
             if ring else f"M{o},0 A{o},{o} 0 0 1 0,{o} L0,0 Z")
        return f'<rect width="{box}" height="{box}" fill="#000" fill-opacity="0"/><path d="{d}" {style(fill_cls)}/>'

    mid = 4
    if part == "top":
        art = f'<rect width="{mid}" height="{box}" fill="#000" fill-opacity="0"/>'
        if ring:
            art += f'<rect width="{mid}" height="2" {style(fill_cls)}/>'
        else:
            art += f'<rect width="{mid}" height="{box}" {style(fill_cls)}/>'
        return art
    if part == "bottom":
        art = f'<rect width="{mid}" height="{box}" fill="#000" fill-opacity="0"/>'
        if ring:
            art += f'<rect y="{box - 2}" width="{mid}" height="2" {style(fill_cls)}/>'
        else:
            art += f'<rect width="{mid}" height="{box}" {style(fill_cls)}/>'
        return art
    if part == "left":
        art = f'<rect width="{box}" height="{mid}" fill="#000" fill-opacity="0"/>'
        if ring:
            art += f'<rect width="2" height="{mid}" {style(fill_cls)}/>'
        else:
            art += f'<rect width="{box}" height="{mid}" {style(fill_cls)}/>'
        return art
    if part == "right":
        art = f'<rect width="{box}" height="{mid}" fill="#000" fill-opacity="0"/>'
        if ring:
            art += f'<rect x="{box - 2}" width="2" height="{mid}" {style(fill_cls)}/>'
        else:
            art += f'<rect width="{box}" height="{mid}" {style(fill_cls)}/>'
        return art
    # center
    art = f'<rect width="{mid}" height="{mid}" fill="#000" fill-opacity="0"/>'
    if not ring:
        art += f'<rect width="{mid}" height="{mid}" {style(fill_cls)}/>'
    return art


def gen_button(path, p):
    if not path.exists():
        src = ROOT / "plasma" / "desktoptheme" / "material-dark-kde" / "widgets" / "button.svg"
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(src.read_text())
    text = path.read_text()
    # Palette fallback inside the SVG (KSvg replaces these via ColorScheme-*).
    text = re.sub(
        r'(\.ColorScheme-Text \{ color:)[^;]+(; \})',
        rf'\g<1>{p["text"]}\g<2>', text)
    text = re.sub(
        r'(\.ColorScheme-Highlight \{ color:)[^;]+(; \})',
        rf'\g<1>{p["primary"]}\g<2>', text)
    text = re.sub(
        r'(\.ColorScheme-ButtonBackground \{ color:)[^;]+(; \})',
        rf'\g<1>{p["button"]}\g<2>', text)

    def repl(m):
        gid, transform, _inner = m.group(1), m.group(2) or "", m.group(3)
        family, _, part = gid.rpartition("-")
        if family not in list(BUTTON_FAMILIES) + ["focus", "toolbutton-focus"]:
            return m.group(0)
        if part not in ("topleft", "topright", "bottomleft", "bottomright",
                        "top", "bottom", "left", "right", "center"):
            return m.group(0)
        return f'<g id="{gid}"{transform}>{_btn_art(family, gid, part)}</g>'

    text = re.sub(r'<g id="([^"]+)"( transform="[^"]*")?>(.*?)</g>', repl, text)
    path.write_text(text)
    print("patched", path.relative_to(ROOT))


# ---------------------------------------------------------------------------
# Color scheme and Konsole scheme from the same palette.
# ---------------------------------------------------------------------------
def scheme_text(p):
    t = lambda k: csv(p[k])  # noqa: E731
    fg = f"""DecorationFocus={t('primary')}
DecorationHover={t('primary_container')}
ForegroundActive={t('primary')}
ForegroundInactive={t('inactive')}
ForegroundLink={t('link')}
ForegroundNegative={t('error')}
ForegroundNeutral={t('warning')}
ForegroundNormal={t('text')}
ForegroundPositive={t('positive')}
ForegroundVisited={t('visited')}"""
    header_inactive = f"""DecorationFocus={t('primary')}
DecorationHover={t('primary_container')}
ForegroundActive={t('primary')}
ForegroundInactive={t('wm_inactive')}
ForegroundLink={t('primary_container')}
ForegroundNegative={t('error')}
ForegroundNeutral={t('warning')}
ForegroundNormal={t('inactive')}
ForegroundPositive={t('positive')}
ForegroundVisited={t('visited')}"""
    selection = f"""DecorationFocus={t('primary_container')}
DecorationHover={t('primary_container')}
ForegroundActive={t('on_primary')}
ForegroundInactive={t('sel_fg_inactive')}
ForegroundLink={t('sel_link')}
ForegroundNegative={t('sel_neg')}
ForegroundNeutral={t('sel_neutral')}
ForegroundNormal={t('on_primary')}
ForegroundPositive={t('sel_pos')}
ForegroundVisited={t('sel_visited')}"""
    return f"""[ColorEffects:Disabled]
Color={t('disabled')}
ColorAmount=0
ColorEffect=0
ContrastAmount=0.65
ContrastEffect=1
IntensityAmount=0.1
IntensityEffect=2

[ColorEffects:Inactive]
ChangeSelectionColor=true
Color={t('inactive_effect')}
ColorAmount=0.025
ColorEffect=2
ContrastAmount=0.1
ContrastEffect=2
Enable=false
IntensityAmount=0
IntensityEffect=0

[Colors:Button]
BackgroundAlternate={t('button_alt')}
BackgroundNormal={t('button')}
{fg}

[Colors:Complementary]
BackgroundAlternate={t('section')}
BackgroundNormal={t('window')}
{fg}

[Colors:Header]
BackgroundAlternate={t('section')}
BackgroundNormal={t('window')}
{fg}

[Colors:Header][Inactive]
BackgroundAlternate={t('window')}
BackgroundNormal={t('header_inactive')}
{header_inactive}

[Colors:Selection]
BackgroundAlternate={t('primary_container')}
BackgroundNormal={t('primary')}
{selection}

[Colors:Tooltip]
BackgroundAlternate={t('button_alt')}
BackgroundNormal={t('tooltip')}
DecorationFocus={t('primary')}
DecorationHover={t('primary')}
ForegroundActive={t('primary')}
ForegroundInactive={t('inactive')}
ForegroundLink={t('primary_container')}
ForegroundNegative={t('error')}
ForegroundNeutral={t('warning')}
ForegroundNormal={t('text')}
ForegroundPositive={t('positive')}
ForegroundVisited={t('visited')}

[Colors:View]
BackgroundAlternate={t('view_alt')}
BackgroundNormal={t('view')}
{fg}

[Colors:Window]
BackgroundAlternate={t('section')}
BackgroundNormal={t('window')}
{fg}

[General]
ColorScheme={p['scheme']}
Name={p['name']}
shadeSortColumn=true

[KDE]
contrast=2

[WM]
activeBackground={t('titlebar')}
activeBlend={t('text')}
activeForeground={t('text')}
inactiveBackground={t('window')}
inactiveBlend={t('wm_inactive')}
inactiveForeground={t('wm_inactive')}
"""


def konsole_text(p):
    t = lambda k: csv(p[k])  # noqa: E731
    colors = []
    for i, (normal, faint, intense) in enumerate(p["ansi"]):
        colors.append(f"[Color{i}]\nColor={csv(normal)}\n\n[Color{i}Faint]\nColor={csv(faint)}\n\n[Color{i}Intense]\nColor={csv(intense)}\n")
    return f"""[Background]
Color={t('konsole_bg')}

[BackgroundFaint]
Color={t('konsole_faint')}

[BackgroundIntense]
Color={t('konsole_intense')}

""" + "\n".join(colors) + f"""
[Foreground]
Color={t('text')}

[ForegroundFaint]
Color={t('inactive')}

[ForegroundIntense]
Color=255,255,255

[General]
Anchor=0.5,0.5
Blur=false
ColorRandomization=false
Description={p['name']}
FillStyle=Tile
Opacity=1
Wallpaper=
"""


def gen_schemes(p):
    scheme_path = ROOT / "color-schemes" / f"{p['scheme']}.colors"
    scheme_path.write_text(scheme_text(p))
    print("wrote", scheme_path.relative_to(ROOT))

    konsole_path = ROOT / "konsole" / f"{p['scheme']}.colorscheme"
    konsole_path.write_text(konsole_text(p))
    print("wrote", konsole_path.relative_to(ROOT))

    theme = ROOT / "plasma" / "desktoptheme" / p["id"]
    (theme / "colors").write_text(scheme_text(p))
    meta = {
        "KPlugin": {
            "Authors": [{"Name": p["name"]}],
            "Description": p["desc"],
            "Id": p["id"],
            "License": "MIT",
            "Name": p["name"],
            "Version": VERSION,
        }
    }
    (theme / "metadata.json").write_text(json.dumps(meta, indent=4) + "\n")
    print("wrote", (theme / "metadata.json").relative_to(ROOT))


def generate(p):
    theme = ROOT / "plasma" / "desktoptheme" / p["id"]
    widgets = theme / "widgets"
    dialogs = theme / "dialogs"
    solid = theme / "solid" / "widgets"
    gen_backgrounds(p, widgets, dialogs)
    gen_viewitem(p, widgets)
    gen_bar(widgets / "bar_meter_horizontal.svg", p)
    gen_bar(widgets / "bar_meter_vertical.svg", p, vertical=True)
    gen_switch(p, widgets)
    gen_lineedit(p, widgets)
    gen_slider(p, widgets)
    gen_panel(p, widgets, solid)
    gen_button(widgets / "button.svg", p)
    gen_schemes(p)


if __name__ == "__main__":
    wanted = sys.argv[1:] or list(PALETTES)
    for name in wanted:
        if name not in PALETTES:
            raise SystemExit(f"Varian tidak dikenal: {name} (pilih: {', '.join(PALETTES)})")
        generate(PALETTES[name])
