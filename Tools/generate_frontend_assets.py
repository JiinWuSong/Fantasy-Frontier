from __future__ import annotations

import math
import random
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont


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


def shift_color(color: tuple[int, int, int], delta: int) -> tuple[int, int, int]:
    return tuple(max(0, min(255, channel + delta)) for channel in color)


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


def add_sun(image: Image.Image, center: tuple[int, int], radius: int) -> None:
    glow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(glow)
    x, y = center
    for scale, alpha in ((4.8, 38), (3.5, 54), (2.2, 82), (1.2, 138)):
        size = int(radius * scale)
        draw.ellipse((x - size, y - size, x + size, y + size), fill=(255, 242, 201, alpha))
    glow = glow.filter(ImageFilter.GaussianBlur(radius=42))
    image.alpha_composite(glow)

    core = Image.new("RGBA", image.size, (0, 0, 0, 0))
    core_draw = ImageDraw.Draw(core)
    core_draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=(255, 249, 230, 240))
    core_draw.ellipse((x - radius // 2, y - radius // 2, x + radius // 2, y + radius // 2), fill=(255, 255, 244, 214))
    image.alpha_composite(core.filter(ImageFilter.GaussianBlur(radius=2)))


def add_clouds(image: Image.Image, rng: random.Random, count: int, top_band: tuple[int, int]) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    width, _ = image.size
    for _ in range(count):
        x = rng.randint(-120, width - 180)
        y = rng.randint(top_band[0], top_band[1])
        cloud_w = rng.randint(220, 620)
        cloud_h = rng.randint(70, 170)
        alpha = rng.randint(48, 90)
        color = rng.choice(((255, 255, 255, alpha), (255, 246, 228, alpha), (239, 250, 255, alpha)))
        for _ in range(rng.randint(6, 11)):
            dx = rng.randint(0, cloud_w)
            dy = rng.randint(0, cloud_h)
            rx = rng.randint(cloud_w // 7, cloud_w // 3)
            ry = rng.randint(cloud_h // 2, cloud_h)
            draw.ellipse((x + dx - rx, y + dy - ry, x + dx + rx, y + dy + ry), fill=color)
    image.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(radius=24)))


def make_hill_points(width: int, height: int, horizon: int, variance: int, step: int, seed: int) -> list[tuple[int, int]]:
    rng = random.Random(seed)
    points = [(0, height)]
    for x in range(0, width + step, step):
        wave = math.sin(x * 0.004 + rng.random() * 0.8) * variance * 0.5
        lift = math.cos(x * 0.0023 + rng.random() * 1.1) * variance * 0.22
        y = int(horizon + wave + lift + rng.randint(-variance // 4, variance // 4))
        points.append((x, y))
    points.append((width, height))
    return points


def add_mountains(image: Image.Image) -> None:
    draw = ImageDraw.Draw(image)
    ranges = [
        ((112, 155, 186, 255), 475, 70, 110, 5),
        ((87, 134, 163, 255), 535, 82, 100, 8),
        ((70, 122, 126, 255), 612, 64, 88, 11),
    ]
    for color, horizon, variance, step, seed in ranges:
        draw.polygon(make_hill_points(WIDTH, HEIGHT, horizon, variance, step, seed), fill=color)

    snow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    snow_draw = ImageDraw.Draw(snow)
    snow_draw.polygon([(1140, 474), (1220, 380), (1300, 470)], fill=(239, 248, 252, 145))
    snow_draw.polygon([(1280, 495), (1370, 390), (1470, 505)], fill=(242, 249, 255, 130))
    image.alpha_composite(snow.filter(ImageFilter.GaussianBlur(radius=2)))


def add_haze_bands(image: Image.Image) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    for y, alpha, height in ((420, 46, 120), (560, 38, 92), (690, 28, 72)):
        draw.rounded_rectangle((100, y, WIDTH - 120, y + height), radius=60, fill=(244, 240, 228, alpha))
    image.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(radius=32)))


def draw_round_tree(draw: ImageDraw.ImageDraw, x: int, baseline: int, scale: float, trunk: tuple[int, int, int, int], leaves: tuple[int, int, int, int], rng: random.Random) -> None:
    trunk_w = max(6, int(18 * scale))
    trunk_h = int(150 * scale)
    draw.rounded_rectangle((x - trunk_w // 2, baseline - trunk_h, x + trunk_w // 2, baseline), radius=trunk_w // 2, fill=trunk)
    for _ in range(int(14 * scale) + 8):
        rx = int(rng.uniform(44, 92) * scale)
        ry = int(rng.uniform(34, 74) * scale)
        dx = rng.randint(-60, 60)
        dy = rng.randint(-128, -18)
        tone = rng.randint(-16, 18)
        color = (*shift_color(leaves[:3], tone), leaves[3])
        draw.ellipse((x + dx - rx, baseline + dy - ry, x + dx + rx, baseline + dy + ry), fill=color)


def draw_pine(draw: ImageDraw.ImageDraw, x: int, baseline: int, scale: float, trunk: tuple[int, int, int, int], leaves: tuple[int, int, int, int], rng: random.Random) -> None:
    trunk_w = max(4, int(14 * scale))
    trunk_h = int(160 * scale)
    draw.rounded_rectangle((x - trunk_w // 2, baseline - trunk_h, x + trunk_w // 2, baseline), radius=trunk_w // 2, fill=trunk)
    for index in range(6):
        width = int((108 - index * 12) * scale)
        y = baseline - int((index + 1) * 28 * scale)
        draw.polygon(
            [
                (x - width, y + 16),
                (x, y - int(54 * scale)),
                (x + width, y + 16),
            ],
            fill=(*shift_color(leaves[:3], rng.randint(-18, 18)), leaves[3]),
        )


def add_stream(image: Image.Image) -> None:
    stream = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(stream)
    path = [
        (0, 870),
        (280, 822),
        (570, 792),
        (940, 760),
        (1320, 720),
        (WIDTH, 684),
        (WIDTH, 840),
        (1260, 872),
        (850, 930),
        (430, 965),
        (0, 985),
    ]
    draw.polygon(path, fill=(106, 196, 201, 188))
    for offset in range(0, 14):
        alpha = 24 - offset
        draw.line((40, 915 + offset * 3, WIDTH - 80, 708 + offset * 3), fill=(255, 255, 255, alpha), width=2)
    image.alpha_composite(stream.filter(ImageFilter.GaussianBlur(radius=4)))


def draw_hut(draw: ImageDraw.ImageDraw, x: int, y: int, scale: float) -> None:
    body = (129, 88, 52, 255)
    roof = (88, 59, 37, 255)
    trim = (194, 160, 105, 255)
    width = int(110 * scale)
    height = int(70 * scale)
    draw.rounded_rectangle((x, y, x + width, y + height), radius=int(12 * scale), fill=body)
    draw.polygon(
        [
            (x - int(10 * scale), y + int(8 * scale)),
            (x + width // 2, y - int(44 * scale)),
            (x + width + int(10 * scale), y + int(8 * scale)),
        ],
        fill=roof,
    )
    draw.rectangle((x + int(40 * scale), y + int(26 * scale), x + int(66 * scale), y + height), fill=(70, 49, 30, 255))
    draw.rectangle((x + int(16 * scale), y + int(24 * scale), x + int(34 * scale), y + int(42 * scale)), fill=(255, 223, 142, 156))
    draw.rectangle((x + int(74 * scale), y + int(24 * scale), x + int(92 * scale), y + int(42 * scale)), fill=(255, 229, 157, 144))
    draw.line((x + int(12 * scale), y + height + int(3 * scale), x + width - int(12 * scale), y + height + int(3 * scale)), fill=trim, width=max(1, int(3 * scale)))


def draw_kobold(draw: ImageDraw.ImageDraw, x: int, y: int, scale: float, facing: int = 1) -> None:
    body = (91, 145, 112, 255)
    cloth = (122, 74, 150, 255)
    outline = (40, 58, 46, 255)
    head = [
        (x + facing * int(-10 * scale), y - int(14 * scale)),
        (x + facing * int(-2 * scale), y - int(34 * scale)),
        (x + facing * int(10 * scale), y - int(18 * scale)),
        (x + facing * int(22 * scale), y - int(34 * scale)),
        (x + facing * int(28 * scale), y - int(14 * scale)),
        (x + facing * int(10 * scale), y + int(6 * scale)),
    ]
    draw.polygon(head, fill=body, outline=outline)
    draw.rounded_rectangle((x, y, x + int(30 * scale), y + int(34 * scale)), radius=int(8 * scale), fill=cloth, outline=outline)
    draw.rectangle((x + int(4 * scale), y + int(34 * scale), x + int(10 * scale), y + int(54 * scale)), fill=outline)
    draw.rectangle((x + int(18 * scale), y + int(34 * scale), x + int(24 * scale), y + int(54 * scale)), fill=outline)
    draw.line((x + int(30 * scale), y + int(14 * scale), x + int(46 * scale), y + int(8 * scale)), fill=outline, width=max(1, int(3 * scale)))


def draw_fox(draw: ImageDraw.ImageDraw, x: int, y: int, scale: float) -> None:
    fur = (226, 132, 79, 255)
    light = (255, 238, 214, 255)
    dark = (96, 58, 42, 255)
    draw.ellipse((x, y - int(24 * scale), x + int(72 * scale), y + int(8 * scale)), fill=fur)
    draw.ellipse((x + int(54 * scale), y - int(30 * scale), x + int(84 * scale), y - int(2 * scale)), fill=fur)
    draw.polygon(
        [
            (x + int(62 * scale), y - int(18 * scale)),
            (x + int(68 * scale), y - int(40 * scale)),
            (x + int(76 * scale), y - int(20 * scale)),
        ],
        fill=dark,
    )
    draw.polygon(
        [
            (x + int(74 * scale), y - int(16 * scale)),
            (x + int(83 * scale), y - int(36 * scale)),
            (x + int(87 * scale), y - int(14 * scale)),
        ],
        fill=dark,
    )
    draw.polygon(
        [
            (x - int(6 * scale), y - int(6 * scale)),
            (x - int(38 * scale), y - int(34 * scale)),
            (x - int(10 * scale), y - int(14 * scale)),
        ],
        fill=fur,
    )
    draw.polygon(
        [
            (x - int(34 * scale), y - int(30 * scale)),
            (x - int(55 * scale), y - int(18 * scale)),
            (x - int(20 * scale), y - int(4 * scale)),
        ],
        fill=light,
    )
    for leg in range(4):
        offset = x + int((10 + leg * 14) * scale)
        draw.rectangle((offset, y, offset + int(5 * scale), y + int(24 * scale)), fill=dark)
    draw.ellipse((x + int(62 * scale), y - int(20 * scale), x + int(66 * scale), y - int(16 * scale)), fill=(30, 20, 16, 255))


def draw_wolf(draw: ImageDraw.ImageDraw, x: int, y: int, scale: float) -> None:
    fur = (129, 139, 151, 255)
    dark = (72, 80, 88, 255)
    light = (201, 210, 216, 255)
    draw.ellipse((x, y - int(28 * scale), x + int(88 * scale), y + int(10 * scale)), fill=fur)
    draw.ellipse((x + int(62 * scale), y - int(32 * scale), x + int(98 * scale), y + int(0 * scale)), fill=fur)
    draw.polygon([(x + int(70 * scale), y - int(18 * scale)), (x + int(77 * scale), y - int(42 * scale)), (x + int(83 * scale), y - int(16 * scale))], fill=dark)
    draw.polygon([(x + int(83 * scale), y - int(16 * scale)), (x + int(92 * scale), y - int(38 * scale)), (x + int(96 * scale), y - int(14 * scale))], fill=dark)
    draw.polygon([(x - int(4 * scale), y - int(10 * scale)), (x - int(36 * scale), y - int(30 * scale)), (x - int(6 * scale), y - int(4 * scale))], fill=dark)
    draw.line((x + int(74 * scale), y - int(12 * scale), x + int(96 * scale), y - int(6 * scale)), fill=dark, width=max(1, int(3 * scale)))
    draw.line((x + int(98 * scale), y - int(8 * scale), x + int(108 * scale), y - int(6 * scale)), fill=dark, width=max(1, int(2 * scale)))
    for leg in range(4):
        offset = x + int((12 + leg * 16) * scale)
        draw.rectangle((offset, y, offset + int(5 * scale), y + int(28 * scale)), fill=dark)
    draw.ellipse((x + int(18 * scale), y - int(4 * scale), x + int(50 * scale), y + int(10 * scale)), fill=light)


def add_day_forest(image: Image.Image) -> None:
    draw = ImageDraw.Draw(image)
    draw.polygon(make_hill_points(WIDTH, HEIGHT, 670, 48, 96, 20), fill=(80, 143, 104, 255))
    draw.polygon(make_hill_points(WIDTH, HEIGHT, 742, 52, 86, 33), fill=(65, 119, 86, 255))
    draw.polygon(make_hill_points(WIDTH, HEIGHT, 810, 46, 74, 44), fill=(53, 95, 68, 255))
    add_stream(image)

    rng = random.Random(117)
    for cluster_x in range(-80, WIDTH + 120, 84):
        baseline = rng.randint(630, 735)
        scale = rng.uniform(0.7, 1.35)
        if rng.random() > 0.45:
            draw_round_tree(draw, cluster_x, baseline, scale, (88, 61, 37, 255), (74, 155, 96, 222), rng)
        else:
            draw_pine(draw, cluster_x, baseline, scale, (82, 58, 37, 255), (51, 119, 85, 222), rng)

    village = Image.new("RGBA", image.size, (0, 0, 0, 0))
    village_draw = ImageDraw.Draw(village)
    draw_hut(village_draw, 1168, 696, 0.95)
    draw_hut(village_draw, 1274, 728, 0.76)
    draw_hut(village_draw, 1390, 706, 0.88)
    village_draw.ellipse((1300, 784, 1372, 830), fill=(245, 167, 82, 180))
    village_draw.ellipse((1308, 792, 1364, 824), fill=(255, 221, 137, 182))
    for puff_x, puff_y, r in ((1332, 744, 22), (1354, 718, 28), (1372, 684, 36)):
        village_draw.ellipse((puff_x - r, puff_y - r, puff_x + r, puff_y + r), fill=(255, 250, 245, 68))
    draw_kobold(village_draw, 1210, 780, 0.92, 1)
    draw_kobold(village_draw, 1340, 792, 0.84, -1)
    draw_kobold(village_draw, 1450, 776, 0.96, 1)
    image.alpha_composite(village.filter(ImageFilter.GaussianBlur(radius=1)))

    wildlife = Image.new("RGBA", image.size, (0, 0, 0, 0))
    wildlife_draw = ImageDraw.Draw(wildlife)
    draw_wolf(wildlife_draw, 760, 824, 0.92)
    draw_wolf(wildlife_draw, 880, 838, 0.74)
    draw_fox(wildlife_draw, 412, 874, 0.86)
    image.alpha_composite(wildlife)


def add_sun_rays(image: Image.Image) -> None:
    rays = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(rays)
    anchors = [(258, 132), (324, 152), (378, 172), (432, 162), (510, 180)]
    for index, (x, y) in enumerate(anchors):
        spread = 250 + index * 60
        draw.polygon(
            [
                (x, y),
                (x + 72, y + 12),
                (x + spread + 300, HEIGHT),
                (x - spread, HEIGHT),
            ],
            fill=(255, 241, 197, 32),
        )
    image.alpha_composite(rays.filter(ImageFilter.GaussianBlur(radius=28)))


def add_pollen(image: Image.Image, count: int, seed: int) -> None:
    rng = random.Random(seed)
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    for _ in range(count):
        x = rng.randint(0, WIDTH - 1)
        y = rng.randint(180, HEIGHT - 50)
        radius = rng.choice((2, 2, 3, 4))
        color = rng.choice(((255, 236, 186, 110), (255, 245, 222, 132), (212, 244, 208, 86)))
        draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=color)
    image.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(radius=3)))


def add_foreground_frame(image: Image.Image) -> None:
    overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)

    trunks = [(76, 110, 874), (96, 132, 1080), (1744, 1786, 1080), (1672, 1714, 980)]
    for x0, x1, y1 in trunks:
        draw.rounded_rectangle((x0, 0, x1, y1), radius=20, fill=(94, 61, 36, 214))

    for start_x in (48, 120, 1820, 1740):
        direction = 1 if start_x < WIDTH // 2 else -1
        for branch in range(6):
            sy = 120 + branch * 102
            ex = start_x + direction * (150 + branch * 18)
            ey = sy + (-40 if branch % 2 == 0 else 52)
            draw.line((start_x, sy, ex, ey), fill=(103, 70, 44, 196), width=10)

    leaf_overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
    leaf_draw = ImageDraw.Draw(leaf_overlay)
    rng = random.Random(42)
    for anchor_x in (170, 248, 1650, 1755):
        for _ in range(36):
            lx = anchor_x + rng.randint(-140, 120)
            ly = rng.randint(50, 460)
            rw = rng.randint(20, 52)
            rh = rng.randint(12, 32)
            color = rng.choice(((92, 175, 90, 172), (132, 205, 108, 164), (78, 150, 109, 180)))
            leaf_draw.ellipse((lx - rw, ly - rh, lx + rw, ly + rh), fill=color)
    overlay.alpha_composite(leaf_overlay.filter(ImageFilter.GaussianBlur(radius=4)))

    meadow = Image.new("RGBA", image.size, (0, 0, 0, 0))
    meadow_draw = ImageDraw.Draw(meadow)
    meadow_draw.polygon(
        [(0, 820), (240, 790), (520, 860), (820, 816), (1120, 870), (1388, 824), (1676, 880), (WIDTH, 842), (WIDTH, HEIGHT), (0, HEIGHT)],
        fill=(52, 93, 61, 218),
    )
    for x in range(0, WIDTH, 12):
        height = 58 + int(20 * math.sin(x * 0.04))
        meadow_draw.line((x, 920, x + 8, 920 - height), fill=(86, 138, 88, 160), width=2)
    for flower_x, flower_y, color in (
        (302, 904, (244, 196, 104, 255)),
        (484, 936, (232, 146, 176, 255)),
        (1522, 914, (245, 214, 138, 255)),
        (1642, 944, (206, 170, 245, 255)),
    ):
        meadow_draw.ellipse((flower_x - 6, flower_y - 6, flower_x + 6, flower_y + 6), fill=color)
        meadow_draw.line((flower_x, flower_y, flower_x, flower_y + 22), fill=(78, 122, 72, 255), width=2)
    overlay.alpha_composite(meadow)

    wildlife = Image.new("RGBA", image.size, (0, 0, 0, 0))
    wildlife_draw = ImageDraw.Draw(wildlife)
    draw_fox(wildlife_draw, 208, 920, 1.12)
    draw_wolf(wildlife_draw, 1480, 922, 0.94)
    overlay.alpha_composite(wildlife)

    image.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(radius=0.5)))


def build_background_layers() -> None:
    def add_cloud_bank(image: Image.Image) -> None:
        bank = Image.new("RGBA", image.size, (0, 0, 0, 0))
        draw = ImageDraw.Draw(bank)
        for x, y, w, h, alpha in (
            (-120, 250, 820, 220, 88),
            (320, 210, 860, 240, 76),
            (980, 220, 920, 230, 86),
            (1400, 270, 720, 190, 70),
        ):
            draw.ellipse((x, y, x + w, y + h), fill=(255, 241, 222, alpha))
            draw.ellipse((x + 40, y + 18, x + w - 10, y + h + 22), fill=(248, 253, 255, max(34, alpha - 22)))
        image.alpha_composite(bank.filter(ImageFilter.GaussianBlur(radius=42)))

    def add_reflective_lake(image: Image.Image) -> None:
        water = Image.new("RGBA", image.size, (0, 0, 0, 0))
        draw = ImageDraw.Draw(water)
        shoreline = [
            (0, 706),
            (232, 694),
            (438, 712),
            (640, 690),
            (896, 702),
            (1116, 678),
            (1360, 694),
            (1612, 672),
            (WIDTH, 686),
            (WIDTH, 920),
            (1540, 902),
            (1226, 910),
            (884, 948),
            (560, 960),
            (188, 934),
            (0, 944),
        ]
        draw.polygon(shoreline, fill=(133, 197, 214, 178))
        for index in range(18):
            alpha = max(10, 44 - index * 2)
            y = 738 + index * 14
            draw.line((120, y, WIDTH - 120, y - 24), fill=(255, 246, 229, alpha), width=3)
        draw.rounded_rectangle((0, 650, WIDTH, 760), radius=44, fill=(255, 225, 182, 28))
        image.alpha_composite(water.filter(ImageFilter.GaussianBlur(radius=5)))

    def add_ruin_silhouettes(image: Image.Image) -> None:
        ruins = Image.new("RGBA", image.size, (0, 0, 0, 0))
        draw = ImageDraw.Draw(ruins)
        left_color = (76, 105, 120, 210)
        right_color = (82, 112, 132, 210)
        draw.polygon([(166, 664), (214, 558), (238, 558), (252, 664)], fill=left_color)
        draw.polygon([(214, 644), (308, 530), (332, 530), (318, 644)], fill=left_color)
        draw.rectangle((196, 616, 352, 664), fill=left_color)
        draw.rectangle((420, 584, 442, 664), fill=(58, 84, 98, 210))
        draw.rectangle((442, 538, 466, 664), fill=left_color)
        draw.rectangle((470, 602, 528, 664), fill=(58, 84, 98, 210))

        draw.rectangle((1460, 562, 1490, 664), fill=right_color)
        draw.rectangle((1512, 522, 1544, 664), fill=right_color)
        draw.rectangle((1562, 548, 1590, 664), fill=right_color)
        draw.polygon([(1620, 664), (1660, 586), (1702, 586), (1738, 664)], fill=right_color)
        draw.polygon([(1504, 664), (1588, 604), (1696, 604), (1762, 664)], fill=(67, 95, 112, 210))

        halo = Image.new("RGBA", image.size, (0, 0, 0, 0))
        halo_draw = ImageDraw.Draw(halo)
        halo_draw.ellipse((1380, 518, 1720, 698), fill=(123, 214, 233, 34))
        halo_draw.ellipse((120, 526, 548, 714), fill=(117, 199, 228, 30))
        ruins.alpha_composite(halo.filter(ImageFilter.GaussianBlur(radius=30)))
        image.alpha_composite(ruins.filter(ImageFilter.GaussianBlur(radius=0.7)))

    sky = make_vertical_gradient((WIDTH, HEIGHT), (98, 170, 235), (252, 244, 219))
    add_sun(sky, (1498, 166), 90)
    add_clouds(sky, random.Random(15), 18, (44, 238))
    add_cloud_bank(sky)
    add_mountains(sky)
    add_haze_bands(sky)
    horizon_glow = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    horizon_draw = ImageDraw.Draw(horizon_glow)
    horizon_draw.rounded_rectangle((-40, 470, WIDTH + 40, 720), radius=120, fill=(255, 230, 188, 44))
    sky.alpha_composite(horizon_glow.filter(ImageFilter.GaussianBlur(radius=48)))
    sky.save(SLATE_DIR / "MenuSky.png")

    mid = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(mid)
    draw.polygon(make_hill_points(WIDTH, HEIGHT, 590, 54, 92, 63), fill=(104, 147, 165, 218))
    draw.polygon(make_hill_points(WIDTH, HEIGHT, 642, 42, 84, 71), fill=(78, 128, 112, 255))
    draw.polygon(make_hill_points(WIDTH, HEIGHT, 694, 36, 76, 77), fill=(58, 107, 78, 255))
    add_reflective_lake(mid)
    add_ruin_silhouettes(mid)

    tree_band = Image.new("RGBA", mid.size, (0, 0, 0, 0))
    tree_draw = ImageDraw.Draw(tree_band)
    rng = random.Random(117)
    for x in range(-20, WIDTH + 30, 18):
        height = rng.randint(42, 124)
        width = rng.randint(10, 24)
        base_y = 742 + int(18 * math.sin(x * 0.03))
        color = rng.choice(((42, 86, 64, 210), (54, 102, 72, 205), (66, 118, 84, 196)))
        tree_draw.polygon([(x - width, base_y), (x, base_y - height), (x + width, base_y)], fill=color)
    for x in range(-40, WIDTH + 60, 64):
        radius_x = rng.randint(56, 96)
        radius_y = rng.randint(28, 58)
        center_y = rng.randint(660, 772)
        tree_draw.ellipse((x - radius_x, center_y - radius_y, x + radius_x, center_y + radius_y), fill=(73, 128, 88, 54))
    mid.alpha_composite(tree_band.filter(ImageFilter.GaussianBlur(radius=4)))

    glow_grove = Image.new("RGBA", mid.size, (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow_grove)
    for x, y, rx, ry, color in (
        (420, 706, 160, 70, (198, 243, 214, 28)),
        (980, 684, 210, 82, (255, 231, 186, 24)),
        (1452, 710, 170, 76, (185, 226, 240, 30)),
    ):
        glow_draw.ellipse((x - rx, y - ry, x + rx, y + ry), fill=color)
    mid.alpha_composite(glow_grove.filter(ImageFilter.GaussianBlur(radius=26)))

    add_pollen(mid, 58, 88)
    mid = mid.filter(ImageFilter.GaussianBlur(radius=0.8))
    mid.save(SLATE_DIR / "MenuMidground.png")

    mist = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    add_sun_rays(mist)
    add_haze_bands(mist)
    add_pollen(mist, 76, 91)
    mist_glow = Image.new("RGBA", mist.size, (0, 0, 0, 0))
    mist_draw = ImageDraw.Draw(mist_glow)
    mist_draw.ellipse((1188, 560, 1794, 928), fill=(255, 240, 214, 28))
    mist_draw.ellipse((272, 512, 906, 874), fill=(212, 244, 228, 18))
    mist.alpha_composite(mist_glow.filter(ImageFilter.GaussianBlur(radius=38)))
    mist.save(SLATE_DIR / "MenuMist.png")

    foreground = Image.new("RGBA", (WIDTH, HEIGHT), (0, 0, 0, 0))
    frame = ImageDraw.Draw(foreground)
    frame.polygon([(0, 860), (210, 824), (482, 866), (812, 838), (1130, 886), (1422, 850), (1708, 902), (WIDTH, 872), (WIDTH, HEIGHT), (0, HEIGHT)], fill=(55, 98, 67, 214))
    for x in range(0, WIDTH, 16):
        base_y = 956 + int(18 * math.sin(x * 0.032))
        tip_y = base_y - 50 - int(20 * math.cos(x * 0.052))
        frame.line((x, base_y, x + 8, tip_y), fill=(87, 148, 96, 144), width=2)
    edge_foliage = Image.new("RGBA", foreground.size, (0, 0, 0, 0))
    edge_draw = ImageDraw.Draw(edge_foliage)
    for anchor_x in (74, 178, 1702, 1812):
        for _ in range(14):
            lx = anchor_x + rng.randint(-96, 94)
            ly = rng.randint(40, 420)
            rw = rng.randint(52, 114)
            rh = rng.randint(20, 52)
            edge_draw.ellipse((lx - rw, ly - rh, lx + rw, ly + rh), fill=rng.choice(((86, 162, 93, 58), (122, 196, 108, 52), (72, 135, 99, 66))))
    foreground.alpha_composite(edge_foliage.filter(ImageFilter.GaussianBlur(radius=14)))

    ruin_frame = Image.new("RGBA", foreground.size, (0, 0, 0, 0))
    ruin_draw = ImageDraw.Draw(ruin_frame)
    ruin_draw.rectangle((1702, 520, 1746, 930), fill=(76, 92, 104, 152))
    ruin_draw.rectangle((1628, 604, 1702, 650), fill=(86, 104, 116, 138))
    ruin_draw.ellipse((1604, 594, 1726, 726), outline=(210, 223, 228, 48), width=4)
    foreground.alpha_composite(ruin_frame.filter(ImageFilter.GaussianBlur(radius=1.8)))
    add_pollen(foreground, 34, 77)
    foreground.save(SLATE_DIR / "MenuForeground.png")

    preview = sky.copy()
    preview.alpha_composite(mid)
    preview.alpha_composite(mist)
    preview.alpha_composite(foreground)
    preview.save(SLATE_DIR / "MenuBackground.png")


def make_menu_surface(size: tuple[int, int], top: tuple[int, int, int], bottom: tuple[int, int, int], glow: tuple[int, int, int], seed: int) -> Image.Image:
    rng = random.Random(seed)
    width, height = size
    image = make_vertical_gradient(size, top, bottom)

    cloud_layer = Image.new("RGBA", size, (0, 0, 0, 0))
    cloud_draw = ImageDraw.Draw(cloud_layer)
    for _ in range(28):
        x = rng.randint(-80, width - 20)
        y = rng.randint(-40, height - 20)
        w = rng.randint(width // 9, width // 3)
        h = rng.randint(height // 8, height // 3)
        tint = rng.randint(-10, 18)
        color = (*shift_color(glow, tint), rng.randint(18, 42))
        cloud_draw.ellipse((x, y, x + w, y + h), fill=color)
    image.alpha_composite(cloud_layer.filter(ImageFilter.GaussianBlur(radius=28)))

    sheen = Image.new("RGBA", size, (0, 0, 0, 0))
    sheen_draw = ImageDraw.Draw(sheen)
    sheen_draw.rounded_rectangle((18, 18, width - 18, height // 2), radius=26, fill=(255, 255, 255, 18))
    sheen_draw.rounded_rectangle((18, height // 2, width - 18, height - 18), radius=26, fill=(0, 0, 0, 14))
    image.alpha_composite(sheen.filter(ImageFilter.GaussianBlur(radius=20)))

    vignette = Image.new("RGBA", size, (0, 0, 0, 0))
    vignette_draw = ImageDraw.Draw(vignette)
    vignette_draw.rounded_rectangle((0, 0, width, height), radius=34, outline=(14, 24, 28, 95), width=12)
    image.alpha_composite(vignette.filter(ImageFilter.GaussianBlur(radius=10)))
    return image


def add_trim(base: Image.Image, inset: int, color: tuple[int, int, int, int], inner: tuple[int, int, int, int]) -> None:
    draw = ImageDraw.Draw(base)
    width, height = base.size
    draw.rounded_rectangle((inset, inset, width - inset, height - inset), radius=28, outline=color, width=6)
    draw.rounded_rectangle((inset + 10, inset + 10, width - inset - 10, height - inset - 10), radius=22, outline=inner, width=2)


def add_vines(base: Image.Image, seed: int) -> None:
    rng = random.Random(seed)
    overlay = Image.new("RGBA", base.size, (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    width, height = base.size
    starts = [(18, 26), (width - 40, 34), (28, height - 34), (width - 36, height - 38)]
    for start_x, start_y in starts:
        direction = 1 if start_x < width // 2 else -1
        points = [(start_x, start_y)]
        for step_index in range(1, 8):
            points.append((start_x + direction * rng.randint(12, 28) * step_index, start_y + rng.randint(-4, 22) * step_index))
        draw.line(points, fill=(82, 126, 66, 120), width=4)
        for px, py in points[1:]:
            leaf_color = rng.choice(((108, 169, 85, 122), (92, 146, 88, 118), (138, 188, 98, 104)))
            draw.ellipse((px - 10, py - 5, px + 10, py + 5), fill=leaf_color)
    base.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(radius=2)))


def build_panels() -> None:
    def add_corner_filigree(image: Image.Image, color: tuple[int, int, int, int]) -> None:
        overlay = Image.new("RGBA", image.size, (0, 0, 0, 0))
        draw = ImageDraw.Draw(overlay)
        width, height = image.size
        for x0, y0, x1, y1, sx, sy in (
            (42, 38, 120, 112, 1, 1),
            (width - 120, 38, width - 42, 112, -1, 1),
            (42, height - 112, 120, height - 38, 1, -1),
            (width - 120, height - 112, width - 42, height - 38, -1, -1),
        ):
            draw.line((x0, y0, x1, y0), fill=color, width=4)
            draw.line((x0, y0, x0, y1), fill=color, width=4)
            draw.arc((min(x0, x1) - 20, min(y0, y1) - 20, max(x0, x1) + 20, max(y0, y1) + 20), 180 if sx > 0 else 270, 270 if sy > 0 else 0, fill=color, width=3)
            draw.ellipse((x0 + sx * 18 - 6, y0 + sy * 18 - 6, x0 + sx * 18 + 6, y0 + sy * 18 + 6), fill=(255, 242, 210, 120))
        image.alpha_composite(overlay.filter(ImageFilter.GaussianBlur(radius=0.5)))

    panel = make_menu_surface((920, 560), (61, 103, 140), (21, 35, 60), (158, 213, 220), 21)
    shadow = Image.new("RGBA", panel.size, (0, 0, 0, 0))
    shadow_draw = ImageDraw.Draw(shadow)
    shadow_draw.rounded_rectangle((12, 18, 908, 548), radius=36, fill=(0, 0, 0, 68))
    panel.alpha_composite(shadow.filter(ImageFilter.GaussianBlur(radius=16)))
    add_trim(panel, 14, (226, 197, 128, 255), (255, 240, 208, 186))
    add_corner_filigree(panel, (234, 213, 160, 196))
    panel.save(SLATE_DIR / "MenuPanel.png")

    variants = [
        ("MenuButton.png", (74, 129, 156), (31, 55, 89), (167, 226, 229), (235, 205, 138, 255), (255, 244, 214, 176)),
        ("MenuButtonHover.png", (98, 159, 180), (42, 72, 110), (205, 239, 235), (255, 226, 158, 255), (255, 250, 229, 196)),
        ("MenuButtonPressed.png", (56, 103, 134), (25, 44, 72), (132, 188, 206), (214, 184, 122, 255), (245, 228, 194, 160)),
    ]
    for index, (filename, top, bottom, glow, trim, inner) in enumerate(variants):
        button = make_menu_surface((640, 156), top, bottom, glow, 30 + index)
        sheen = Image.new("RGBA", button.size, (0, 0, 0, 0))
        sheen_draw = ImageDraw.Draw(sheen)
        sheen_draw.rounded_rectangle((18, 12, 622, 60), radius=18, fill=(255, 255, 255, 20))
        button.alpha_composite(sheen.filter(ImageFilter.GaussianBlur(radius=18)))
        add_trim(button, 10, trim, inner)
        add_corner_filigree(button, (240, 220, 170, 88))
        button.save(SLATE_DIR / filename)

    divider = Image.new("RGBA", (1600, 40), (0, 0, 0, 0))
    draw = ImageDraw.Draw(divider)
    draw.polygon(
        [(16, 20), (96, 10), (1504, 10), (1584, 20), (1504, 30), (96, 30)],
        fill=(231, 196, 112, 255),
    )
    draw.line((124, 20, 1476, 20), fill=(255, 238, 194, 215), width=2)
    draw.ellipse((42, 10, 68, 30), fill=(126, 176, 104, 185))
    draw.ellipse((1532, 10, 1558, 30), fill=(126, 176, 104, 185))
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
    width = 1660
    height = 320
    canvas = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    draw = ImageDraw.Draw(canvas)
    text = "FANTASY FRONTIER"
    tracking = 2
    font_size = 126
    font = ImageFont.truetype(str(FONT_PATH), font_size)
    while text_width(draw, text, font, tracking) > width - 180 and font_size > 90:
        font_size -= 4
        font = ImageFont.truetype(str(FONT_PATH), font_size)
    total_width = text_width(draw, text, font, tracking)
    x = (width - total_width) // 2
    y = 66

    fill = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    fill_draw = ImageDraw.Draw(fill)
    for py in range(height):
        t = clamp(py / height)
        color = mix_color((255, 249, 226), (210, 162, 86), t)
        fill_draw.line((0, py, width, py), fill=(*color, 255))
    crest = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    crest_draw = ImageDraw.Draw(crest)
    crest_draw.ellipse((692, 16, 964, 238), outline=(255, 238, 196, 64), width=6)
    crest_draw.arc((620, 20, 1036, 250), 202, 340, fill=(255, 233, 186, 96), width=5)
    crest_draw.polygon([(728, 206), (828, 150), (938, 206)], fill=(255, 238, 196, 30))
    crest_draw.line((318, 214, 566, 214), fill=(235, 202, 130, 166), width=4)
    crest_draw.line((1094, 214, 1342, 214), fill=(235, 202, 130, 166), width=4)
    crest = crest.filter(ImageFilter.GaussianBlur(radius=6))
    fill.alpha_composite(crest)

    mask = Image.new("L", (width, height), 0)
    mask_draw = ImageDraw.Draw(mask)
    draw_tracked_text(mask_draw, (x, y), text, font, tracking, fill=255, stroke_fill=255, stroke_width=5)

    text_layer = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    text_layer.paste(fill, (0, 0), mask)

    stroke = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    stroke_draw = ImageDraw.Draw(stroke)
    draw_tracked_text(stroke_draw, (x, y), text, font, tracking, fill=(0, 0, 0, 0), stroke_fill=(112, 72, 31, 255), stroke_width=7)

    glow = Image.new("RGBA", (width, height), (0, 0, 0, 0))
    glow_draw = ImageDraw.Draw(glow)
    draw_tracked_text(glow_draw, (x, y), text, font, tracking, fill=(255, 244, 205, 104), stroke_fill=(255, 226, 160, 136), stroke_width=7)
    glow = glow.filter(ImageFilter.GaussianBlur(radius=18))

    canvas.alpha_composite(glow)
    canvas.alpha_composite(stroke)
    canvas.alpha_composite(text_layer)
    canvas.save(SLATE_DIR / "TitleLogo.png")


def build_icon() -> None:
    size = 512
    icon = make_vertical_gradient((size, size), (101, 172, 231), (246, 238, 214))
    add_sun(icon, (392, 110), 46)

    clouds = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    cloud_draw = ImageDraw.Draw(clouds)
    cloud_draw.ellipse((-20, 88, 220, 194), fill=(255, 240, 220, 76))
    cloud_draw.ellipse((124, 62, 382, 186), fill=(246, 252, 255, 58))
    cloud_draw.ellipse((286, 92, 548, 198), fill=(255, 241, 216, 64))
    icon.alpha_composite(clouds.filter(ImageFilter.GaussianBlur(radius=18)))

    scene = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    draw = ImageDraw.Draw(scene)
    draw.polygon([(0, 286), (92, 252), (186, 276), (290, 234), (404, 270), (512, 236), (512, 512), (0, 512)], fill=(99, 152, 172, 190))
    draw.polygon([(0, 332), (74, 316), (186, 326), (284, 310), (372, 322), (512, 304), (512, 512), (0, 512)], fill=(129, 198, 212, 150))
    draw.polygon([(0, 386), (106, 344), (208, 372), (316, 334), (432, 374), (512, 348), (512, 512), (0, 512)], fill=(62, 118, 82, 255))
    draw.polygon([(0, 432), (96, 392), (218, 426), (316, 388), (430, 430), (512, 406), (512, 512), (0, 512)], fill=(46, 90, 64, 255))
    draw.rounded_rectangle((40, 40, 472, 472), radius=92, outline=(236, 200, 118, 255), width=12)
    draw.rounded_rectangle((56, 56, 456, 456), radius=76, outline=(255, 239, 194, 160), width=3)
    draw.rectangle((120, 218, 138, 322), fill=(86, 112, 126, 210))
    draw.rectangle((374, 236, 392, 336), fill=(86, 112, 126, 210))
    draw.polygon([(210, 330), (252, 252), (294, 330)], fill=(255, 244, 220, 140))
    icon.alpha_composite(scene)

    emblem = Image.new("RGBA", (size, size), (0, 0, 0, 0))
    emblem_draw = ImageDraw.Draw(emblem)
    emblem_draw.ellipse((146, 88, 366, 298), outline=(255, 239, 198, 84), width=6)
    emblem_draw.text((126, 126), "FF", font=ImageFont.truetype(str(FONT_PATH), 126), fill=(255, 249, 228, 255), stroke_fill=(102, 62, 27, 255), stroke_width=5)
    icon.alpha_composite(emblem.filter(ImageFilter.GaussianBlur(radius=0.4)))

    icon.save(ICON_PATH, sizes=[(256, 256), (128, 128), (64, 64), (48, 48), (32, 32), (16, 16)])


def main() -> None:
    ensure_dirs()
    build_background_layers()
    build_panels()
    build_logo()
    build_icon()


if __name__ == "__main__":
    main()
