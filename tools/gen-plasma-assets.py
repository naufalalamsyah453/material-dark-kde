#!/usr/bin/env python3
# Generate the Plasma Style SVGs for Material Dark from one palette.
# SPDX-License-Identifier: MIT
"""Regenerate 9-slice / 3-slice Plasma assets with the Material Dark palette.

Usage: python3 tools/gen-plasma-assets.py
"""
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
WIDGETS = ROOT / "plasma" / "desktoptheme" / "material-dark-kde" / "widgets"
DIALOGS = ROOT / "plasma" / "desktoptheme" / "material-dark-kde" / "dialogs"

# Material 3 dark palette (pink seed)
PRIMARY = "#F2B8C6"          # #F2B8C6
ON_PRIMARY = "#2B1219"
PRIMARY_CONTAINER = "#FFD9E2"
SURFACE = "#100E11"
SURFACE_LOW = "#1A1619"
SURFACE_CONTAINER = "#211C1F"
SURFACE_HIGH = "#2C2528"
SURFACE_HIGHEST = "#3A3034"
TEXT = "#F0E4E7"
OUTLINE = "#554A4F"
OUTLINE_VARIANT = "#3A3034"
TOOLTIP = "#322A2E"
PANEL = "#161114"
FIELD = "#1A1619"


def rect(x, y, w, h, color, opacity=None):
    extra = f' fill-opacity="{opacity}"' if opacity is not None else ""
    return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="{color}"{extra}/>'


def rr_parts(prefix, x, y, w, h, r, color):
    """9-slice pieces of a filled rounded rectangle."""
    return [
        f'<g id="{prefix}-topleft" transform="translate({x},{y})"><path d="M0,{r} A{r},{r} 0 0 1 {r},0 L{r},{r} Z" fill="{color}"/></g>',
        f'<g id="{prefix}-topright" transform="translate({x + w - r},{y})"><path d="M0,0 A{r},{r} 0 0 1 {r},{r} L0,{r} Z" fill="{color}"/></g>',
        f'<g id="{prefix}-bottomleft" transform="translate({x},{y + h - r})"><path d="M0,0 A{r},{r} 0 0 0 {r},{r} L{r},0 Z" fill="{color}"/></g>',
        f'<g id="{prefix}-bottomright" transform="translate({x + w - r},{y + h - r})"><path d="M{r},0 A{r},{r} 0 0 1 0,{r} L0,0 Z" fill="{color}"/></g>',
        f'<g id="{prefix}-top" transform="translate({x + r},{y})">' + rect(0, 0, w - 2 * r, r, color) + "</g>",
        f'<g id="{prefix}-bottom" transform="translate({x + r},{y + h - r})">' + rect(0, 0, w - 2 * r, r, color) + "</g>",
        f'<g id="{prefix}-left" transform="translate({x},{y + r})">' + rect(0, 0, r, h - 2 * r, color) + "</g>",
        f'<g id="{prefix}-right" transform="translate({x + w - r},{y + r})">' + rect(0, 0, r, h - 2 * r, color) + "</g>",
        f'<g id="{prefix}-center" transform="translate({x + r},{y + r})">' + rect(0, 0, w - 2 * r, h - 2 * r, color) + "</g>",
    ]


def frame(prefix, x, y, w, h, r, fill, border=None, bw=2):
    """9-slice rounded rectangle, optionally with a real border ring.

    Every piece contains both the border and the fill: an overlay frame with
    different element names would be ignored by KSvg.
    """
    if not border:
        return rr_parts(prefix, x, y, w, h, r, fill)

    ir = max(1, r - bw)
    # The inner arc keeps the outer corner centre and shrinks its radius by the
    # border width, so the ring is evenly thick along both edges.
    tl_outer = f"M0,{r} A{r},{r} 0 0 1 {r},0 L{r},{r} Z"
    tl_inner = f"M{bw},{r} A{ir},{ir} 0 0 1 {r},{bw} L{r},{r} Z"
    tr_outer = f"M0,0 A{r},{r} 0 0 1 {r},{r} L0,{r} Z"
    tr_inner = f"M0,{bw} A{ir},{ir} 0 0 1 {r - bw},{r} L0,{r} Z"
    bl_outer = f"M0,0 A{r},{r} 0 0 0 {r},{r} L{r},0 Z"
    bl_inner = f"M{bw},0 A{ir},{ir} 0 0 0 {r},{r - bw} L{r},0 Z"
    br_outer = f"M{r},0 A{r},{r} 0 0 1 0,{r} L0,0 Z"
    br_inner = f"M{r - bw},0 A{ir},{ir} 0 0 1 0,{r - bw} L0,0 Z"

    def corner(name, px, py, outer, inner):
        return (
            f'<g id="{name}" transform="translate({px},{py})">'
            f'<path d="{outer}" fill="{border}"/>'
            f'<path d="{inner}" fill="{fill}"/>'
            "</g>"
        )

    return [
        corner(prefix + "-topleft", x, y, tl_outer, tl_inner),
        corner(prefix + "-topright", x + w - r, y, tr_outer, tr_inner),
        corner(prefix + "-bottomleft", x, y + h - r, bl_outer, bl_inner),
        corner(prefix + "-bottomright", x + w - r, y + h - r, br_outer, br_inner),
        f'<g id="{prefix}-top" transform="translate({x + r},{y})">'
        + rect(0, 0, w - 2 * r, bw, border)
        + rect(0, bw, w - 2 * r, r - bw, fill)
        + "</g>",
        f'<g id="{prefix}-bottom" transform="translate({x + r},{y + h - r})">'
        + rect(0, 0, w - 2 * r, r - bw, fill)
        + rect(0, r - bw, w - 2 * r, bw, border)
        + "</g>",
        f'<g id="{prefix}-left" transform="translate({x},{y + r})">'
        + rect(0, 0, bw, h - 2 * r, border)
        + rect(bw, 0, r - bw, h - 2 * r, fill)
        + "</g>",
        f'<g id="{prefix}-right" transform="translate({x + w - r},{y + r})">'
        + rect(0, 0, r - bw, h - 2 * r, fill)
        + rect(r - bw, 0, bw, h - 2 * r, border)
        + "</g>",
        f'<g id="{prefix}-center" transform="translate({x + r},{y + r})">' + rect(0, 0, w - 2 * r, h - 2 * r, fill) + "</g>",
    ]


def hint_margins(prefix, x, y, dx, dy, extra=None):
    pts = [
        (prefix + "-hint-left-margin", x, y, dx, dy),
        (prefix + "-hint-right-margin", x + 40, y, dx, dy),
        (prefix + "-hint-top-margin", x, y + 20, dx, dy),
        (prefix + "-hint-bottom-margin", x, y + 40, dx, dy),
    ]
    out = []
    for name, px, py, pw, ph in pts:
        out.append(f'<g id="{name}">' + rect(px, py, pw, ph, "#000", "0") + "</g>")
    if extra:
        out += extra
    return out


def write_svg(path, width, height, body):
    path.parent.mkdir(parents=True, exist_ok=True)
    text = '<?xml version="1.0" encoding="UTF-8"?>\n'
    text += f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" height="{height}">\n'
    text += "\n".join(body)
    text += "\n</svg>\n"
    path.write_text(text)
    print("wrote", path.relative_to(ROOT))


def simple_frame(path, radius, fill, natural=64, margins=12, prefix="normal"):
    # No hint margins: the corner radius itself is the content padding, which
    # keeps the 9-slice drawing aligned. A hint larger than the corner size
    # leaves visible notches between the corner and the stretched edges.
    body = frame(prefix, 10, 10, natural, natural, radius, fill)
    write_svg(path, natural + 30, natural + 30, body)


def gen_backgrounds():
    # Applet / popup surface (raised container)
    simple_frame(WIDGETS / "background.svg", 12, SURFACE_CONTAINER)
    # Dialog surface: slightly darker, corners a bit larger
    simple_frame(DIALOGS / "background.svg", 16, SURFACE_CONTAINER)
    # Tooltip: dark elevated surface, small radius
    simple_frame(WIDGETS / "tooltip.svg", 8, TOOLTIP)


def gen_viewitem():
    w, h, r = 48, 40, 8
    states = [
        ("normal", 10, None, "0"),
        ("hover", 70, TEXT, "0.08"),
        ("selected", 130, PRIMARY, None),
        ("selected+hover", 190, PRIMARY_CONTAINER, None),
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
            # opacity only for the hover state
            if opacity:
                for i in range(len(body) - 9, len(body)):
                    body[i] = body[i].replace('fill="' + color + '"', f'fill="{color}" fill-opacity="{opacity}"')
    body.append('<g id="hint-tile-center">' + rect(10, 260, 8, 8, "#000", "0") + "</g>")
    write_svg(WIDGETS / "viewitem.svg", 90, 300, body)


def cap_left(x, y, r, color):
    return f'<g id="CAPID-left" transform="translate({x},{y})"><path d="M{r},0 A{r},{r} 0 0 0 {r},{2 * r} L{r},0 Z" fill="{color}"/><rect x="0" y="{r}" width="{r}" height="0.001" fill="{color}"/></g>'


def pill3(prefix, x, y, width, height, color):
    r = height / 2
    r = int(r) if r == int(r) else r
    return [
        f'<g id="{prefix}-left" transform="translate({x},{y})"><path d="M{r},0 A{r},{r} 0 0 0 {r},{height} L{r},0 Z" fill="{color}"/></g>',
        f'<g id="{prefix}-center" transform="translate({x + r},{y})">' + rect(0, 0, width, height, color) + "</g>",
        f'<g id="{prefix}-right" transform="translate({x + r + width},{y})"><path d="M0,0 A{r},{r} 0 0 1 0,{height} L0,0 Z" fill="{color}"/></g>',
    ]


def gen_bar(path, vertical=False):
    height = 12
    r = height // 2
    body = []
    body += pill3("bar-inactive", 10, 10, 28, height, SURFACE_HIGHEST)
    body += pill3("bar-active", 10, 40, 28, height, PRIMARY)
    if vertical:
        # vertical meter: same assets but 12px wide bars
        pass
    body.append('<g id="hint-bar-size">' + rect(10, 70, 40, height, "#000", "0") + "</g>")
    body.append('<g id="hint-tile-center">' + rect(10, 90, 4, 4, "#000", "0") + "</g>")
    body.append('<g id="hint-stretch-borders">' + rect(10, 96, 4, 4, "#000", "0") + "</g>")
    write_svg(path, 90, 120, body)


def gen_switch():
    h = 22
    r = h // 2
    body = []
    body += pill3("inactive", 10, 10, 22, h, SURFACE_HIGHEST)
    body += pill3("active", 10, 42, 22, h, PRIMARY)

    # Handle: 24x24 cell, 19px dark knob with a pink ring so it reads on
    # both the dark and the pink track.
    def handle(name, y, fill, ring, ring_opacity=1.0):
        return (
            f'<g id="{name}" transform="translate(10,{y})">'
            + rect(0, 0, 24, 24, "#000", "0")
            + f'<circle cx="12" cy="12" r="8.5" fill="{fill}" stroke="{ring}" stroke-width="2" stroke-opacity="{ring_opacity}"/>'
            + "</g>"
        )

    body.append(handle("handle", 80, ON_PRIMARY, PRIMARY))
    body.append(handle("handle-active", 110, ON_PRIMARY, PRIMARY))
    body.append(handle("handle-hover", 140, ON_PRIMARY, PRIMARY_CONTAINER))
    body.append(handle("handle-pressed", 170, ON_PRIMARY, PRIMARY_CONTAINER))
    body.append(
        '<g id="handle-focus" transform="translate(10,200)">'
        + rect(0, 0, 32, 32, "#000", "0")
        + f'<circle cx="16" cy="16" r="13" fill="none" stroke="{PRIMARY}" stroke-width="2" stroke-opacity="0.7"/>'
        + "</g>"
    )
    body.append('<g id="handle-shadow" transform="translate(10,240)">' + rect(0, 0, 24, 24, "#000", "0") + "</g>")
    body.append('<g id="hint-bar-size">' + rect(10, 270, 44, h, "#000", "0") + "</g>")
    body.append('<g id="hint-stretch-borders">' + rect(10, 296, 4, 4, "#000", "0") + "</g>")
    write_svg(WIDGETS / "switch.svg", 90, 320, body)


def gen_lineedit():
    w, h, r, bw = 64, 36, 10, 2
    rows = [
        ("base", 10, FIELD, OUTLINE),
        ("hover", 70, FIELD, PRIMARY_CONTAINER),
        ("focus", 130, FIELD, PRIMARY),
        ("focusframe", 190, FIELD, PRIMARY),
    ]
    body = []
    for name, y, fill, border in rows:
        body += frame(name, 10, y, w, h, r, fill, border, bw)
    write_svg(WIDGETS / "lineedit.svg", 100, 250, body)


def gen_slider():
    body = []
    body.append('<g id="hint-handle-size">' + rect(10, 10, 20, 20, "#000", "0") + "</g>")
    for direction, y in (("horizontal", 36), ("vertical", 200)):
        body.append(
            f'<g id="{direction}-slider-handle" transform="translate(10,{y})">'
            + rect(0, 0, 20, 20, "#000", "0")
            + f'<circle cx="10" cy="10" r="8" fill="{ON_PRIMARY}" stroke="{PRIMARY}" stroke-width="2"/>'
            + "</g>"
        )
        body.append(f'<g id="{direction}-slider-shadow" transform="translate(10,{y + 24})">' + rect(0, 0, 20, 20, "#000", "0") + "</g>")
        body.append(
            f'<g id="{direction}-slider-hover" transform="translate(10,{y + 48})">'
            + rect(0, 0, 36, 36, "#000", "0")
            + f'<circle cx="18" cy="18" r="17" fill="none" stroke="{PRIMARY}" stroke-width="2" stroke-opacity="0.45"/>'
            + "</g>"
        )
        body.append(
            f'<g id="{direction}-slider-focus" transform="translate(10,{y + 88})">'
            + rect(0, 0, 36, 36, "#000", "0")
            + f'<circle cx="18" cy="18" r="17" fill="none" stroke="{PRIMARY}" stroke-width="2" stroke-opacity="0.75"/>'
            + "</g>"
        )
    # Groove: 12px tall rounded track (4px caps), same structure as the
    # original theme so Slider.qml's fixedMargins add up to 12.
    r = 4
    for prefix, y, color in (("groove", 340, SURFACE_HIGHEST), ("groove-highlight", 370, PRIMARY)):
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
    write_svg(WIDGETS / "slider.svg", 90, 400, body)


def gen_panel():
    # Plasma 6 panel-background is used in three forms:
    #   * unprefixed elements     -> non-floating panel (adaptive/translucent)
    #   * prefix "floating"       -> floating panel (18px pill + 18px padding)
    #   * solid/widgets/...       -> opaque panel when a window touches it
    # The unprefixed frame must be flat edge-to-edge; a rounded unprefixed
    # frame would leave gaps at the screen edges.
    w, h, edge = 64, 48, 4
    x, y = 10, 10
    body = [
        f'<g id="topleft">' + rect(x, y, edge, edge, PANEL) + "</g>",
        f'<g id="top">' + rect(x + edge, y, w - 2 * edge, edge, PANEL) + "</g>",
        f'<g id="topright">' + rect(x + w - edge, y, edge, edge, PANEL) + "</g>",
        f'<g id="left">' + rect(x, y + edge, edge, h - 2 * edge, PANEL) + "</g>",
        f'<g id="center">' + rect(x + edge, y + edge, w - 2 * edge, h - 2 * edge, PANEL) + "</g>",
        f'<g id="right">' + rect(x + w - edge, y + edge, edge, h - 2 * edge, PANEL) + "</g>",
        f'<g id="bottomleft">' + rect(x, y + h - edge, edge, edge, PANEL) + "</g>",
        f'<g id="bottom">' + rect(x + edge, y + h - edge, w - 2 * edge, edge, PANEL) + "</g>",
        f'<g id="bottomright">' + rect(x + w - edge, y + h - edge, edge, edge, PANEL) + "</g>",
    ]
    # Padding hints (transparent); size = fixed margin.
    body += [
        '<g id="hint-left-margin">' + rect(x, y + 20, 4, 4, "#000", "0") + "</g>",
        '<g id="hint-right-margin">' + rect(x + w - 4, y + 20, 4, 4, "#000", "0") + "</g>",
        '<g id="hint-top-margin">' + rect(x + 20, y, 4, 4, "#000", "0") + "</g>",
        '<g id="hint-bottom-margin">' + rect(x + 20, y + h - 4, 4, 4, "#000", "0") + "</g>",
    ]
    # Floating variant: 18px rounded pill, 18px float padding.
    fx, fy, fw, fh, fr = 10, 80, 64, 48, 18
    body += frame("floating", fx, fy, fw, fh, fr, PANEL)
    body += [
        '<g id="floating-hint-left-margin">' + rect(fx, fy + 24, fr, 4, "#000", "0") + "</g>",
        '<g id="floating-hint-right-margin">' + rect(fx + fw - fr, fy + 24, fr, 4, "#000", "0") + "</g>",
        '<g id="floating-hint-top-margin">' + rect(fx + 24, fy, 4, fr, "#000", "0") + "</g>",
        '<g id="floating-hint-bottom-margin">' + rect(fx + 24, fy + fh - fr, 4, fr, "#000", "0") + "</g>",
    ]
    write_svg(WIDGETS / "panel-background.svg", 110, 190, body)
    # Opaque (solid) variant: Plasma paints this when a window touches the panel.
    write_svg(ROOT / "plasma" / "desktoptheme" / "material-dark-kde" / "solid" / "widgets" / "panel-background.svg", 110, 190, body)


def patch_button():
    path = WIDGETS / "button.svg"
    text = path.read_text()
    text = text.replace("#E6E0E9", TEXT).replace("#D0BCFF", PRIMARY).replace("#4A4458", SURFACE_HIGH)
    # Pressed / checked buttons get a stronger pink state layer.
    start = text.find('<g id="pressed-topleft"')
    end = text.find('<g id="focus-topleft"')
    if start != -1 and end != -1:
        chunk = text[start:end]
        chunk = chunk.replace('class="ColorScheme-Text"', 'class="ColorScheme-Highlight"')
        chunk = chunk.replace("fill-opacity:0.12", "fill-opacity:0.22")
        text = text[:start] + chunk + text[end:]
    path.write_text(text)
    print("patched", path.relative_to(ROOT))


if __name__ == "__main__":
    gen_backgrounds()
    gen_viewitem()
    gen_bar(WIDGETS / "bar_meter_horizontal.svg")
    gen_bar(WIDGETS / "bar_meter_vertical.svg", vertical=True)
    gen_switch()
    gen_lineedit()
    gen_slider()
    gen_panel()
    patch_button()
