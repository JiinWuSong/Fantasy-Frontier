from __future__ import annotations

import math
import random
from pathlib import Path

from PIL import Image, ImageChops, ImageDraw, ImageFilter, ImageFont


ROOT = Path(__file__).resolve().parents[1]
SLATE_DIR = ROOT / "Content" / "Slate"
FONT_PATH = SLATE_DIR / "Fonts" / "Georgia-Bold.ttf"
ICON_PATH = ROOT / "Build" / "Windows" / "Application.ico"

WIDTH = 1920
HEIGHT = 1080


def clamp(value: float, low: float = 0.0, high: float = 1.0) -> float:
    return max(low, min(high, value))


def mix(a: float, b: float, t: float) -> float:
    return a + (b - a) * t


def mix_color(a: tuple[int, int, int], b: tuple[int, int, int], t: float) -> tuple[int, int, int]:
    return (
        int(mix(a[0], b[0], t)),
        int(mix(a[1], b[1], t)),
        int(mix(a[2], b[2], t)),
    )


def ensure_dirs() -> None:
    SLATE_DIR.mkdir(parents=True, exist_ok=True)
    ICON_PATH.parent.mkdir(parents=True, exist_ok=True)


def make_vertical_gradient(size: tuple[int, int], top: tuple[int, int, int], bottom: tuple[int, int, int]) -> Image.Image:
    width, height = size
    image = Image.new("RGBA", size)
    draw = ImageDraw.Draw(image)
    for y in range(height):
        t = y / max(1, height - 1)
        color = mix_color(top, bottom, t)
        draw.line((0, y, width, y), fill=(*color, 255))
    return image


def add_stars(image: Image.Image, rng: random.Random, count: int) -> None:
    draw = ImageDraw.Draw(image)
    width, height = image.size
    for _ in range(count):
        x = rng.randint(0, width - 1)
        y = rng.randint(0, int(height * 0.7))
        brightness = rng.randint(160, 255)
        radius = rng.choice((1, 1, 1, 2, 2, 3))
        draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=(brightness, brightness, brightness, 170))

        if radius >= 2 and rng.random() > 0.7:
            glow = Image.new("RGBA", image.size, (0, 0, 0, 0))
            glow_draw = ImageDraw.Draw(glow)
            glow_draw.line((x - radius * 6, y, x + radius * 6, y), fill=(255, 245, 215, 80), width=1)
            glow_draw.line((x, y - radius * 6, x, y + radius * 6), fill=(255, 245, 215, 80), width=1)
            image.alpha_composite(glow.filter(ImageFilter.GaussianBlur(radius=2)))


def add_moon(image: Image.Image, center: tuple[int, int], radius: int) -> None:
    glow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    x, y = center
    glow_draw.ellipse((x - radius * 3, y - radius * 3, x + radius * 3, y + radius * 3), fill=(230, 240, 255, 60))
    glow_draw.ellipse((x - radius * 2, y - radius * 2, x + radius * 2, y + radius * 2), fill=(245, 250, 255, 105))
    image.alpha_composite(glow.filter(ImageFilter.GaussianBlur(radius=36)))

    moon = Image.new("RGBA", image.size, (0, 0, 0, 0))
    moon_draw = ImageDraw.Draw(moon)
    moon_draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=(244, 242, 228, 235))
    moon_draw.ellipse((x - radius * 0.8, y - radius * 0.3, x + radius * 0.2, y + radius * 0.9), fill=(218, 218, 214, 60))
    moon_draw.ellipse((x - radius * 0.55, y - radius * 0.8, x + radius * 0.05, y - radius * 0.25), fill=(215, 215, 212, 48))
    image.alpha_composite(moon.filter(ImageFilter.GaussianBlur(radius=1)))


def add_nebula(image: Image.Image, rng: random.Random) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    palette = [
        (38, 122, 169, 80),
        (90, 169, 212, 55),
        (20, 65, 119, 75),
        (127, 206, 255, 42),
    ]
    for _ in range(20):
        x = rng.randint(-150, WIDTH - 80)
        y = rng.randint(20, int(HEIGHT * 0.58))
        w = rng.randint(220, 760)
        h = rng.randint(100, 360)
        color = rng.choice(palette)
        draw.ellipse((x, y, x + w, y + h), fill=color)
    overlay = overlay.filter(ImageFilter.GaussianBlur(radius=48))
    image.alpha_composite(overlay)


def make_mountain_points(rng: random.Random, horizon: int, variance: int, step: int) -> list[tuple[int, int]]:
    points = [(0, HEIGHT)]
    for x in range(0, WIDTH + step, step):
        wave = math.sin(x * 0.003 + rng.random() * 0.7) * variance * 0.32
        y = int(horizon + rng.randint(-variance, variance) * 0.55 + wave)
        points.append((x, y))
    points.append((WIDTH, HEIGHT))
    return points


def add_landscape(base: Image.Image, rng: random.Random) -> None:
    draw = ImageDraw.Draw(base)

    layers = [
        ((18, 35, 58, 255), 500, 110, 180),
        ((20, 46, 64, 255), 590, 90, 150),
        ((12, 30, 42, 255), 690, 75, 120),
        ((8, 20, 30, 255), 790, 65, 100),
        ((5, 14, 21, 255), 900, 52, 80),
    ]
    for color, horizon, variance, step in layers:
        points = make_mountain_points(rng, horizon, variance, step)
        draw.polygon(points, fill=color)

    lake = Image.new("RGBA", base.size, (0, 0, 0, 0))
    lake_draw = ImageDraw.Draw(lake)
    lake_draw.polygon(
        [
            (0, 645),
            (430, 628),
            (930, 640),
            (1350, 615),
            (1920, 626),
            (1920, 730),
            (0, 730),
        ],
        fill=(28, 114, 132, 82),
    )
    base.alpha_composite(lake.filter(ImageFilter.GaussianBlur(radius=4)))

    mist = Image.new("RGBA", base.size, (0, 0, 0, 0))
    mist_draw = ImageDraw.Draw(mist)
    for y, alpha in ((560, 32), (610, 46), (670, 30)):
        mist_draw.rounded_rectangle((90, y, 1830, y + 78), radius=42, fill=(195, 222, 235, alpha))
    base.alpha_composite(mist.filter(ImageFilter.GaussianBlur(radius=28)))

    ruin = Image.new("RGBA", base.size, (0, 0, 0, 0))
    ruin_draw = ImageDraw.Draw(ruin)
    ruin_draw.polygon([(1180, 540), (1245, 500), (1315, 540), (1300, 700), (1200, 700)], fill=(11, 18, 26, 210))
    ruin_draw.rectangle((1228, 568, 1265, 700), fill=(22, 36, 48, 255))
    ruin_draw.polygon([(1238, 486), (1246, 445), (1258, 484)], fill=(18, 24, 30, 220))
    ruin_draw.rectangle((1206, 580, 1225, 700), fill=(15, 24, 31, 245))
    ruin_draw.rectangle((1270, 596, 1290, 700), fill=(15, 24, 31, 245))
    base.alpha_composite(ruin)


def add_fireflies(image: Image.Image, rng: random.Random, region_top: int, region_bottom: int, count: int) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    for _ in range(count):
        x = rng.randint(40, WIDTH - 40)
        y = rng.randint(region_top, region_bottom)
        radius = rng.choice((2, 3, 4))
        color = rng.choice(((255, 226, 129, 140), (179, 255, 208, 115), (255, 244, 180, 130)))
        draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=color)
    image.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(radius=4)))


def make_tree(draw: ImageDraw.ImageDraw, rng: random.Random, x: int, baseline: int, scale: float, color: tuple[int, int, int, int]) -> None:
    trunk_h = int(130 * scale)
    trunk_w = max(4, int(12 * scale))
    draw.rectangle((x - trunk_w // 2, baseline - trunk_h, x + trunk_w // 2, baseline), fill=color)
    branches = int(6 * scale) + 3
    for index in range(branches):
        y = baseline - int((index + 1) * trunk_h / (branches + 1))
        width = int((trunk_h * 0.9 - index * trunk_h * 0.08) * 0.9)
        jitter = rng.randint(-8, 8)
        draw.polygon(
            [
                (x - width // 2 + jitter, y + 6),
                (x + jitter, y - int(52 * scale)),
                (x + width // 2 + jitter, y + 6),
            ],
            fill=color,
        )


def add_creature(draw: ImageDraw.ImageDraw, x: int, y: int, scale: float, kind: str, color: tuple[int, int, int, int]) -> None:
    if kind == "elk":
        draw.ellipse((x, y - 16 * scale, x + 55 * scale, y + 12 * scale), fill=color)
        draw.ellipse((x + 42 * scale, y - 18 * scale, x + 67 * scale, y + 0 * scale), fill=color)
        for leg in range(4):
            offset = x + 8 * scale + leg * 13 * scale
            draw.rectangle((offset, y + 6 * scale, offset + 4 * scale, y + 26 * scale), fill=color)
        draw.line((x + 58 * scale, y - 15 * scale, x + 73 * scale, y - 28 * scale), fill=color, width=max(1, int(3 * scale)))
        draw.line((x + 60 * scale, y - 16 * scale, x + 72 * scale, y - 6 * scale), fill=color, width=max(1, int(3 * scale)))
    elif kind == "goblin":
        draw.ellipse((x + 10 * scale, y - 22 * scale, x + 34 * scale, y + 0 * scale), fill=color)
        draw.rectangle((x + 8 * scale, y - 8 * scale, x + 38 * scale, y + 20 * scale), fill=color)
        draw.polygon([(x + 5 * scale, y - 14 * scale), (x + 14 * scale, y - 34 * scale), (x + 20 * scale, y - 12 * scale)], fill=color)
        draw.polygon([(x + 28 * scale, y - 12 * scale), (x + 37 * scale, y - 33 * scale), (x + 42 * scale, y - 10 * scale)], fill=color)
        for leg in range(2):
            offset = x + 12 * scale + leg * 12 * scale
            draw.rectangle((offset, y + 18 * scale, offset + 4 * scale, y + 35 * scale), fill=color)
        draw.line((x + 2 * scale, y + 2 * scale, x - 14 * scale, y + 8 * scale), fill=color, width=max(1, int(3 * scale)))
    else:
        draw.ellipse((x + 8 * scale, y - 20 * scale, x + 42 * scale, y + 2 * scale), fill=color)
        draw.rectangle((x + 4 * scale, y - 2 * scale, x + 44 * scale, y + 20 * scale), fill=color)
        draw.ellipse((x + 36 * scale, y - 18 * scale, x + 58 * scale, y + 2 * scale), fill=color)
        for leg in range(2):
            offset = x + 10 * scale + leg * 18 * scale
            draw.rectangle((offset, y + 18 * scale, offset + 5 * scale, y + 36 * scale), fill=color)
        draw.line((x + 55 * scale, y - 10 * scale, x + 68 * scale, y - 20 * scale), fill=color, width=max(1, int(4 * scale)))


def build_background_layers() -> None:
    rng = random.Random(11)
    sky = make_vertical_gradient((WIDTH, HEIGHT), (4, 16, 32), (17, 63, 90))
    add_nebula(sky, rng)
    add_stars(sky, rng, 520)
    add_moon(sky, (1545, 198), 74)
    add_landscape(sky, rng)
    add_fireflies(sky, rng, 540, 920, 42)
    sky.save(SLATE_DIR / "MenuSky.png")

    mid = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    mid_draw = ImageDraw.Draw(mid)
    mid_draw.polygon(
        [(0, 720), (280, 695), (510, 735), (760, 688), (1120, 722), (1370, 680), (1650, 712), (1920, 692), (1920, 1080), (0, 1080)],
        fill=(6, 18, 24, 220),
    )
    for cluster_x in range(-120, 2050, 70):
        make_tree(mid_draw, rng, cluster_x + rng.randint(-20, 20), 730 + rng.randint(-30, 35), rng.uniform(0.7, 1.45), (8, 18, 23, 235))

    ridge = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    ridge_draw = ImageDraw.Draw(ridge)
    ridge_draw.polygon(
        [(0, 792), (225, 752), (465, 772), (760, 742), (1000, 776), (1302, 734), (1604, 760), (1920, 726), (1920, 1080), (0, 1080)],
        fill=(3, 10, 14, 238),
    )
    ridge_draw.polygon([(1320, 670), (1368, 620), (1405, 642), (1394, 768), (1329, 768)], fill=(5, 11, 16, 235))
    ridge_draw.arc((1342, 598, 1438, 696), start=205, end=330, fill=(5, 11, 16, 230), width=8)
    add_creature(ridge_draw, 780, 770, 1.0, "orc", (3, 8, 12, 255))
    add_creature(ridge_draw, 862, 774, 0.78, "goblin", (3, 8, 12, 255))
    add_creature(ridge_draw, 948, 768, 0.88, "goblin", (3, 8, 12, 255))
    add_creature(ridge_draw, 1190, 760, 1.05, "elk", (4, 10, 14, 255))
    add_creature(ridge_draw, 1460, 754, 0.9, "orc", (3, 8, 12, 255))
    mid.alpha_composite(ridge)
    add_fireflies(mid, rng, 640, 930, 26)
    mid = mid.filter(ImageFilter.GaussianBlur(radius=1))
    mid.save(SLATE_DIR / "MenuMidground.png")

    foreground = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    fg_draw = ImageDraw.Draw(foreground)
    fg_draw.polygon([(0, 845), (240, 818), (430, 868), (640, 830), (920, 880), (1200, 850), (1510, 894), (1920, 850), (1920, 1080), (0, 1080)], fill=(1, 6, 10, 245))
    for x in range(40, 1920, 52):
        height = rng.randint(40, 118)
        fg_draw.polygon([(x, 860), (x + 8, 860 - height), (x + 16, 860)], fill=(8, 20, 18, 210))
    for trunk_x in (120, 170, 280, 1640, 1720, 1810):
        trunk_w = 34 if trunk_x < 400 else 42
        fg_draw.rectangle((trunk_x, 150, trunk_x + trunk_w, 1080), fill=(5, 11, 14, 230))
        for branch in range(6):
            start_y = 250 + branch * 95 + rng.randint(-20, 20)
            direction = -1 if trunk_x < 1000 else 1
            fg_draw.line(
                (
                    trunk_x + trunk_w // 2,
                    start_y,
                    trunk_x + trunk_w // 2 + direction * rng.randint(120, 220),
                    start_y - rng.randint(40, 120),
                ),
                fill=(7, 14, 18, 200),
                width=rng.randint(8, 14),
            )
    add_fireflies(foreground, rng, 520, 980, 24)
    foreground = foreground.filter(ImageFilter.GaussianBlur(radius=0.5))
    foreground.save(SLATE_DIR / "MenuForeground.png")

    mist = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    mist_draw = ImageDraw.Draw(mist)
    for box in ((190, 600, 1770, 760, 48), (120, 710, 1820, 850, 52), (240, 790, 1700, 920, 44)):
        x0, y0, x1, y1, alpha = box
        mist_draw.rounded_rectangle((x0, y0, x1, y1), radius=90, fill=(205, 226, 233, alpha))
    mist = mist.filter(ImageFilter.GaussianBlur(radius=50))
    mist.save(SLATE_DIR / "MenuMist.png")

    preview = sky.copy()
    preview.alpha_composite(mid)
    preview.alpha_composite(mist)
    preview.alpha_composite(foreground)
    preview.save(SLATE_DIR / "MenuBackground.png")


def make_wood_texture(size: tuple[int, int], base: tuple[int, int, int], accent: tuple[int, int, int], seed: int) -> Image.Image:
    rng = random.Random(seed)
    width, height = size
    image = Image.new("RGBA", size, (*base, 255))
    draw = ImageDraw.Draw(image)
    for y in range(height):
        t = y / max(1, height - 1)
        color = mix_color(accent, base, t * 0.45)
        draw.line((0, y, width, y), fill=(*color, 255))
    for x in range(-height, width + height, 18):
        tone = rng.randint(-24, 24)
        alpha = rng.randint(22, 56)
        draw.line((x, 0, x + height, height), fill=(base[0] + tone, base[1] + tone // 2, base[2] + tone // 3, alpha), width=rng.randint(4, 10))
    for y in range(16, height, max(28, height // 4)):
        draw.line((22, y, width - 22, y + rng.randint(-6, 6)), fill=(62, 36, 18, 78), width=3)
    return image.filter(ImageFilter.GaussianBlur(radius=1.2))


def add_trim(base: Image.Image, inset: int, color: tuple[int, int, int, int], inner: tuple[int, int, int, int]) -> None:
    draw = ImageDraw.Draw(base)
    width, height = base.size
    draw.rounded_rectangle((inset, inset, width - inset, height - inset), radius=28, outline=color, width=6)
    draw.rounded_rectangle((inset + 10, inset + 10, width - inset - 10, height - inset - 10), radius=24, outline=inner, width=2)


def add_moss(base: Image.Image, seed: int) -> None:
    rng = random.Random(seed)
    overlay = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    width, height = base.size
    corners = [
        (18, 16, 150, 80),
        (width - 160, 16, width - 12, 76),
        (24, height - 82, 158, height - 10),
        (width - 172, height - 84, width - 18, height - 8),
    ]
    for x0, y0, x1, y1 in corners:
        for _ in range(18):
            x = rng.randint(x0, x1)
            y = rng.randint(y0, y1)
            r = rng.randint(8, 18)
            color = rng.choice(((76, 106, 54, 70), (110, 138, 74, 52), (54, 84, 44, 76)))
            draw.ellipse((x - r, y - r, x + r, y + r), fill=color)
    base.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(radius=6)))


def build_panels() -> None:
    panel = make_wood_texture((920, 560), (48, 31, 21), (82, 52, 33), 21)
    panel_shadow = Image.new("RGBA", panel.size, (0, 0, 0, 0))
    shadow_draw = ImageDraw.Draw(panel_shadow)
    shadow_draw.rounded_rectangle((10, 16, 910, 550), radius=34, fill=(0, 0, 0, 95))
    panel.alpha_composite(panel_shadow.filter(ImageFilter.GaussianBlur(radius=16)))
    add_trim(panel, 14, (161, 121, 64, 255), (214, 185, 128, 170))
    add_moss(panel, 4)
    panel.save(SLATE_DIR / "MenuPanel.png")

    variants = [
        ("MenuButton.png", (90, 57, 34), (138, 92, 55), (186, 152, 91, 255), (226, 202, 160, 160)),
        ("MenuButtonHover.png", (112, 71, 40), (164, 114, 66), (230, 194, 116, 255), (255, 232, 192, 180)),
        ("MenuButtonPressed.png", (66, 42, 26), (104, 70, 44), (138, 107, 62, 255), (195, 165, 114, 130)),
    ]
    for index, (filename, base, accent, trim, inner) in enumerate(variants):
        button = make_wood_texture((640, 156), base, accent, 30 + index)
        add_trim(button, 10, trim, inner)
        add_moss(button, 20 + index)
        button.save(SLATE_DIR / filename)

    divider = Image.new("RGBA", (1600, 40), (0, 0, 0, 0))
    div_draw = ImageDraw.Draw(divider)
    div_draw.polygon(
        [(10, 20), (85, 12), (1515, 12), (1590, 20), (1515, 28), (85, 28)],
        fill=(179, 144, 80, 255),
    )
    div_draw.line((110, 20, 1490, 20), fill=(251, 229, 180, 190), width=2)
    divider = divider.filter(ImageFilter.GaussianBlur(radius=0.6))
    divider.save(SLATE_DIR / "TitleDivider.png")


def text_width(draw: ImageDraw.ImageDraw, text: str, font: ImageFont.FreeTypeFont, tracking: int) -> int:
    width = 0
    for index, char in enumerate(text):
        box = draw.textbbox((0, 0), char, font=font, stroke_width=5)
        width += box[2] - box[0]
        if index < len(text) - 1:
            width += tracking
    return width


def draw_tracked_text(draw: ImageDraw.ImageDraw, origin: tuple[int, int], text: str, font: ImageFont.FreeTypeFont, tracking: int, fill, stroke_fill, stroke_width: int) -> None:
    x, y = origin
    for char in text:
        draw.text((x, y), char, font=font, fill=fill, stroke_fill=stroke_fill, stroke_width=stroke_width)
        box = draw.textbbox((x, y), char, font=font, stroke_width=stroke_width)
        x = box[2] + tracking


def build_logo() -> None:
    width = 1760
    height = 360
    canvas = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(canvas)
    text = "FANTASY FRONTIER"
    tracking = 6
    font_size = 144
    font = ImageFont.truetype(str(FONT_PATH), font_size)
    while text_width(draw, text, font, tracking) > width - 110 and font_size > 96:
        font_size -= 4
        font = ImageFont.truetype(str(FONT_PATH), font_size)
    total_width = text_width(draw, text, font, tracking)
    x = (width - total_width) // 2
    y = max(42, (height - font_size - 58) // 2)

    mask = Image.new("L", (width, height), 0)
    mask_draw = ImageDraw.Draw(mask)
    draw_tracked_text(mask_draw, (x, y), text, font, tracking, fill=255, stroke_fill=255, stroke_width=5)

    fill = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    fill_draw = ImageDraw.Draw(fill)
    for py in range(height):
        t = clamp(py / height)
        color = mix_color((255, 247, 221), (181, 133, 70), t)
        fill_draw.line((0, py, width, py), fill=(*color, 255))

    shine = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    shine_draw = ImageDraw.Draw(shine)
    shine_draw.polygon([(380, 0), (520, 0), (920, height), (780, height)], fill=(255, 255, 255, 76))
    shine = shine.filter(ImageFilter.GaussianBlur(radius=18))
    fill.alpha_composite(shine)

    text_image = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    text_image.paste(fill, (0, 0), mask)

    stroke = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    stroke_draw = ImageDraw.Draw(stroke)
    draw_tracked_text(stroke_draw, (x, y), text, font, tracking, fill=(0, 0, 0, 0), stroke_fill=(63, 35, 12, 255), stroke_width=8)

    glow = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    draw_tracked_text(glow_draw, (x, y), text, font, tracking, fill=(255, 234, 186, 140), stroke_fill=(255, 214, 150, 180), stroke_width=10)
    glow = glow.filter(ImageFilter.GaussianBlur(radius=24))

    shadow = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    shadow_draw = ImageDraw.Draw(shadow)
    draw_tracked_text(shadow_draw, (x + 8, y + 12), text, font, tracking, fill=(12, 7, 4, 210), stroke_fill=(12, 7, 4, 220), stroke_width=8)
    shadow = shadow.filter(ImageFilter.GaussianBlur(radius=9))

    canvas.alpha_composite(glow)
    canvas.alpha_composite(shadow)
    canvas.alpha_composite(stroke)
    canvas.alpha_composite(text_image)

    crest = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    crest_draw = ImageDraw.Draw(crest)
    crest_draw.polygon([(731, 14), (770, 54), (809, 14), (782, 107), (757, 107)], fill=(205, 167, 103, 220))
    crest = crest.filter(ImageFilter.GaussianBlur(radius=0.5))
    canvas.alpha_composite(crest)

    canvas.save(SLATE_DIR / "TitleLogo.png")


def build_icon() -> None:
    size = 512
    icon = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    background = make_vertical_gradient((size, size), (5, 17, 31), (16, 61, 70))
    add_nebula(background, random.Random(90))
    add_moon(background, (388, 120), 34)
    icon.alpha_composite(background)

    frame = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    frame_draw = ImageDraw.Draw(frame)
    frame_draw.rounded_rectangle((24, 24, size - 24, size - 24), radius=88, outline=(208, 174, 101, 255), width=12)
    frame_draw.rounded_rectangle((40, 40, size - 40, size - 40), radius=72, outline=(255, 232, 184, 135), width=3)
    icon.alpha_composite(frame)

    mountain = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    mountain_draw = ImageDraw.Draw(mountain)
    mountain_draw.polygon([(30, 350), (120, 275), (196, 325), (252, 245), (338, 320), (414, 260), (490, 336), (490, 488), (30, 488)], fill=(9, 17, 24, 220))
    mountain_draw.polygon([(38, 396), (134, 338), (226, 388), (310, 318), (410, 382), (486, 344), (486, 488), (38, 488)], fill=(10, 28, 35, 225))
    icon.alpha_composite(mountain)

    sword = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    sword_draw = ImageDraw.Draw(sword)
    sword_draw.polygon([(250, 96), (266, 96), (282, 230), (258, 292), (234, 230)], fill=(236, 229, 216, 255))
    sword_draw.rectangle((212, 250, 304, 272), fill=(188, 151, 84, 255))
    sword_draw.rectangle((246, 272, 270, 374), fill=(132, 87, 41, 255))
    sword_draw.ellipse((212, 364, 304, 454), fill=(198, 164, 100, 255))
    sword = sword.filter(ImageFilter.GaussianBlur(radius=0.4))
    icon.alpha_composite(sword)

    font = ImageFont.truetype(str(FONT_PATH), 124)
    text = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    text_draw = ImageDraw.Draw(text)
    text_draw.text((82, 110), "FF", font=font, fill=(255, 245, 214, 255), stroke_fill=(76, 43, 18, 255), stroke_width=6)
    text = text.filter(ImageFilter.GaussianBlur(radius=0.2))
    icon.alpha_composite(text)

    icon.save(ICON_PATH, sizes=[(256, 256), (128, 128), (64, 64), (48, 48), (32, 32), (16, 16)])


def main() -> None:
    ensure_dirs()
    build_background_layers()
    build_panels()
    build_logo()
    build_icon()


if __name__ == "__main__":
    main()
