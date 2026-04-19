from __future__ import annotations

import math
import shutil
import subprocess
import wave
from array import array
from pathlib import Path

from PIL import Image, ImageDraw, ImageFilter, ImageFont, ImageOps

from generate_frontend_assets import (
    FONT_PATH,
    ROOT,
    SLATE_DIR,
    add_clouds,
    add_sun,
    clamp,
    draw_fox,
    draw_hut,
    draw_kobold,
    draw_wolf,
    draw_tracked_text,
    mix,
    mix_color,
    text_width,
)


VIDEO_WIDTH = 1920
VIDEO_HEIGHT = 1080
PANORAMA_WIDTH = 4800
PANORAMA_HEIGHT = 1800
FPS = 24
DURATION = 12.0
SAMPLE_RATE = 48000

WORK_DIR = ROOT / "Saved" / "GeneratedIntro"
FRAMES_DIR = WORK_DIR / "frames"
AUDIO_PATH = WORK_DIR / "FantasyFrontierIntro.wav"
OUTPUT_VIDEO = ROOT / "Content" / "Movies" / "FantasyFrontierIntro.mp4"
MENU_FRAME_PATH = WORK_DIR / "MenuFrame.png"


def ensure_dirs() -> None:
    FRAMES_DIR.mkdir(parents=True, exist_ok=True)
    OUTPUT_VIDEO.parent.mkdir(parents=True, exist_ok=True)


def smoothstep(a: float, b: float, t: float) -> float:
    if abs(b - a) < 1e-6:
        return 0.0
    x = clamp((t - a) / (b - a))
    return x * x * (3.0 - 2.0 * x)


def lerp_color(a: tuple[int, int, int], b: tuple[int, int, int], t: float) -> tuple[int, int, int]:
    return (
        int(mix(a[0], b[0], t)),
        int(mix(a[1], b[1], t)),
        int(mix(a[2], b[2], t)),
    )


def make_gradient(size: tuple[int, int], top: tuple[int, int, int], bottom: tuple[int, int, int]) -> Image.Image:
    width, height = size
    image = Image.new("RGBA", size)
    draw = ImageDraw.Draw(image)
    for y in range(height):
        t = y / max(1, height - 1)
        draw.line((0, y, width, y), fill=(*lerp_color(top, bottom, t), 255))
    return image


def add_texture(image: Image.Image, amount: int = 20) -> None:
    noise = Image.effect_noise(image.size, amount).convert("L")
    alpha = noise.point(lambda value: int(value * 0.14))
    overlay = Image.new("RGBA", image.size, (255, 255, 255, 0))
    overlay.putalpha(alpha)
    image.alpha_composite(overlay)


def hill_points(width: int, height: int, horizon: int, variance: int, step: int, phase: float) -> list[tuple[int, int]]:
    points = [(0, height)]
    for x in range(0, width + step, step):
        wave = math.sin(x * 0.003 + phase) * variance * 0.55
        wave += math.cos(x * 0.0018 + phase * 0.6) * variance * 0.26
        y = int(horizon + wave)
        points.append((x, y))
    points.append((width, height))
    return points


def draw_tree_mass(draw: ImageDraw.ImageDraw, x: int, baseline: int, scale: float, palette: list[tuple[int, int, int, int]]) -> None:
    trunk = (92, 66, 42, 230)
    trunk_w = max(6, int(24 * scale))
    trunk_h = int(210 * scale)
    draw.rounded_rectangle((x - trunk_w // 2, baseline - trunk_h, x + trunk_w // 2, baseline), radius=trunk_w // 2, fill=trunk)
    for offset in range(18):
        angle = offset * 0.62
        rx = int((78 + math.sin(angle) * 28) * scale)
        ry = int((58 + math.cos(angle * 1.3) * 18) * scale)
        dx = int(math.sin(angle * 1.7) * 90 * scale)
        dy = int(-150 * scale + math.cos(angle) * 50 * scale)
        color = palette[offset % len(palette)]
        draw.ellipse((x + dx - rx, baseline + dy - ry, x + dx + rx, baseline + dy + ry), fill=color)


def draw_pine_mass(draw: ImageDraw.ImageDraw, x: int, baseline: int, scale: float, color: tuple[int, int, int, int]) -> None:
    trunk = (78, 58, 36, 225)
    trunk_w = max(5, int(18 * scale))
    trunk_h = int(232 * scale)
    draw.rounded_rectangle((x - trunk_w // 2, baseline - trunk_h, x + trunk_w // 2, baseline), radius=trunk_w // 2, fill=trunk)
    for index in range(8):
        width = int((130 - index * 12) * scale)
        y = baseline - int((index + 1) * 34 * scale)
        draw.polygon([(x - width, y + 20), (x, y - int(68 * scale)), (x + width, y + 20)], fill=color)


def build_sky_layer() -> Image.Image:
    sky = make_gradient((PANORAMA_WIDTH, PANORAMA_HEIGHT), (100, 175, 234), (240, 249, 255))
    add_sun(sky, (620, 220), 96)
    add_clouds(sky, __import__("random").Random(12), 30, (60, 320))

    mountains = ImageDraw.Draw(sky)
    mountains.polygon(hill_points(PANORAMA_WIDTH, PANORAMA_HEIGHT, 690, 90, 120, 0.2), fill=(150, 188, 211, 255))
    mountains.polygon(hill_points(PANORAMA_WIDTH, PANORAMA_HEIGHT, 765, 112, 110, 1.3), fill=(122, 166, 185, 255))
    mountains.polygon(hill_points(PANORAMA_WIDTH, PANORAMA_HEIGHT, 855, 84, 95, 2.2), fill=(100, 143, 153, 255))
    mountains.polygon([(2910, 808), (3090, 550), (3260, 806)], fill=(238, 247, 252, 130))
    mountains.polygon([(3190, 834), (3364, 592), (3538, 842)], fill=(244, 250, 255, 124))

    haze = Image.new("RGBA", sky.size, (0, 0, 0, 0))
    haze_draw = ImageDraw.Draw(haze)
    for y, alpha, band_h in ((620, 38, 130), (800, 30, 120), (1000, 24, 88)):
        haze_draw.rounded_rectangle((120, y, PANORAMA_WIDTH - 120, y + band_h), radius=90, fill=(247, 240, 224, alpha))
    sky.alpha_composite(haze.filter(ImageFilter.GaussianBlur(radius=42)))
    add_texture(sky, 16)
    return sky.filter(ImageFilter.GaussianBlur(radius=0.4))


def build_far_forest_layer() -> Image.Image:
    image = Image.new("RGBA", (PANORAMA_WIDTH, PANORAMA_HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.polygon(hill_points(PANORAMA_WIDTH, PANORAMA_HEIGHT, 960, 62, 90, 0.7), fill=(82, 145, 106, 255))
    draw.polygon(hill_points(PANORAMA_WIDTH, PANORAMA_HEIGHT, 1070, 56, 84, 1.5), fill=(65, 118, 86, 255))

    palette = [(84, 160, 102, 198), (101, 174, 112, 188), (70, 136, 92, 210), (124, 190, 126, 176)]
    for x in range(-120, PANORAMA_WIDTH + 120, 84):
        if (x // 84) % 3 == 0:
            draw_pine_mass(draw, x, 1060 + (x % 30), 0.82 + ((x % 7) * 0.03), (66, 128, 88, 210))
        else:
            draw_tree_mass(draw, x, 1030 + (x % 34), 0.92 + ((x % 5) * 0.04), palette)

    stream = Image.new("RGBA", image.size, (0, 0, 0, 0))
    stream_draw = ImageDraw.Draw(stream)
    stream_draw.polygon(
        [
            (0, 1290),
            (420, 1210),
            (860, 1160),
            (1340, 1128),
            (1930, 1104),
            (2660, 1060),
            (PANORAMA_WIDTH, 1006),
            (PANORAMA_WIDTH, 1230),
            (2580, 1274),
            (1860, 1312),
            (1160, 1370),
            (520, 1430),
            (0, 1490),
        ],
        fill=(114, 200, 207, 170),
    )
    for idx in range(14):
        alpha = max(8, 26 - idx)
        stream_draw.line((40, 1380 + idx * 4, PANORAMA_WIDTH - 40, 1070 + idx * 4), fill=(255, 255, 255, alpha), width=3)
    image.alpha_composite(stream.filter(ImageFilter.GaussianBlur(radius=4)))

    add_texture(image, 18)
    return image.filter(ImageFilter.GaussianBlur(radius=1.2))


def build_midground_layer() -> Image.Image:
    image = Image.new("RGBA", (PANORAMA_WIDTH, PANORAMA_HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.polygon(hill_points(PANORAMA_WIDTH, PANORAMA_HEIGHT, 1180, 72, 70, 1.0), fill=(54, 101, 70, 255))
    draw.polygon(hill_points(PANORAMA_WIDTH, PANORAMA_HEIGHT, 1290, 66, 68, 1.9), fill=(44, 84, 62, 255))

    palette = [(68, 145, 95, 220), (86, 166, 106, 210), (54, 117, 80, 224), (112, 178, 118, 194)]
    for x in range(-150, PANORAMA_WIDTH + 180, 62):
        baseline = 1240 + int(math.sin(x * 0.01) * 24)
        scale = 1.0 + ((x % 9) * 0.05)
        if x % 180 < 60:
            draw_pine_mass(draw, x, baseline, scale, (47, 96, 69, 236))
        else:
            draw_tree_mass(draw, x, baseline, scale, palette)

    village = Image.new("RGBA", image.size, (0, 0, 0, 0))
    village_draw = ImageDraw.Draw(village)
    village_origin = 2550
    draw_hut(village_draw, village_origin, 1154, 1.18)
    draw_hut(village_draw, village_origin + 160, 1182, 0.96)
    draw_hut(village_draw, village_origin + 312, 1140, 1.04)
    draw_hut(village_draw, village_origin + 468, 1188, 0.92)
    village_draw.ellipse((village_origin + 220, 1280, village_origin + 318, 1346), fill=(248, 175, 84, 188))
    village_draw.ellipse((village_origin + 236, 1288, village_origin + 306, 1338), fill=(255, 225, 146, 186))
    for puff_x, puff_y, radius in ((village_origin + 280, 1218, 28), (village_origin + 302, 1178, 36), (village_origin + 320, 1128, 44)):
        village_draw.ellipse((puff_x - radius, puff_y - radius, puff_x + radius, puff_y + radius), fill=(255, 247, 232, 70))
    image.alpha_composite(village.filter(ImageFilter.GaussianBlur(radius=1.2)))

    add_texture(image, 22)
    return image.filter(ImageFilter.GaussianBlur(radius=0.9))


def build_foreground_layer() -> Image.Image:
    image = Image.new("RGBA", (PANORAMA_WIDTH, PANORAMA_HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(image)
    draw.polygon(
        [
            (0, 1360),
            (460, 1290),
            (920, 1350),
            (1380, 1274),
            (1960, 1354),
            (2680, 1286),
            (3360, 1376),
            (PANORAMA_WIDTH, 1304),
            (PANORAMA_WIDTH, PANORAMA_HEIGHT),
            (0, PANORAMA_HEIGHT),
        ],
        fill=(54, 96, 64, 255),
    )

    for x in range(0, PANORAMA_WIDTH, 18):
        base_y = 1450 + int(math.sin(x * 0.02) * 20)
        tip_y = base_y - 80 - int(math.sin(x * 0.06) * 28)
        draw.line((x, base_y, x + 8, tip_y), fill=(81, 138, 92, 170), width=3)

    trunk_positions = [340, 620, 950, 4720]
    for position in trunk_positions:
        width = 44 if position < 1000 else 52
        draw.rounded_rectangle((position, 0, position + width, PANORAMA_HEIGHT), radius=28, fill=(96, 63, 36, 215))
        for branch in range(7):
            start_y = 200 + branch * 120
            direction = -1 if position > PANORAMA_WIDTH // 2 else 1
            draw.line(
                (
                    position + width // 2,
                    start_y,
                    position + width // 2 + direction * (140 + branch * 18),
                    start_y - 30 + branch * 8,
                ),
                fill=(104, 71, 44, 132),
                width=7,
            )

    leaves = Image.new("RGBA", image.size, (0, 0, 0, 0))
    leaves_draw = ImageDraw.Draw(leaves)
    for anchor in (320, 540, 3920, 4250, 4500):
        for index in range(48):
            angle = index * 0.33
            dx = int(math.cos(angle) * 180)
            dy = int(math.sin(angle * 1.3) * 120)
            x = anchor + dx
            y = 180 + dy + (index % 9) * 12
            rx = 28 + index % 18
            ry = 12 + (index % 6) * 4
            color = [(85, 170, 90, 155), (126, 198, 104, 142), (74, 148, 110, 162)][index % 3]
            leaves_draw.ellipse((x - rx, y - ry, x + rx, y + ry), fill=color)
    image.alpha_composite(leaves.filter(ImageFilter.GaussianBlur(radius=5)))
    add_texture(image, 16)
    return image.filter(ImageFilter.GaussianBlur(radius=0.45))


def build_rays_layer() -> Image.Image:
    rays = Image.new("RGBA", (PANORAMA_WIDTH, PANORAMA_HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(rays)
    for index in range(7):
        x = 430 + index * 70
        draw.polygon(
            [
                (x, 180),
                (x + 80, 210),
                (x + 620 + index * 80, PANORAMA_HEIGHT),
                (x - 300, PANORAMA_HEIGHT),
            ],
            fill=(255, 243, 198, 26),
        )
    haze = Image.new("RGBA", rays.size, (0, 0, 0, 0))
    haze_draw = ImageDraw.Draw(haze)
    for band_y, alpha in ((840, 30), (1040, 26), (1210, 22)):
        haze_draw.rounded_rectangle((160, band_y, PANORAMA_WIDTH - 160, band_y + 140), radius=90, fill=(251, 246, 224, alpha))
    rays.alpha_composite(haze.filter(ImageFilter.GaussianBlur(radius=46)))
    add_texture(rays, 10)
    return rays.filter(ImageFilter.GaussianBlur(radius=12))


def build_particle_layer() -> Image.Image:
    overlay = Image.new("RGBA", (PANORAMA_WIDTH, PANORAMA_HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    rng = __import__("random").Random(44)
    for _ in range(360):
        x = rng.randint(0, PANORAMA_WIDTH - 1)
        y = rng.randint(160, PANORAMA_HEIGHT - 60)
        radius = rng.choice((2, 2, 3, 4))
        color = rng.choice(((255, 235, 182, 92), (255, 247, 230, 112), (211, 245, 206, 72)))
        draw.ellipse((x - radius, y - radius, x + radius, y + radius), fill=color)
    return overlay.filter(ImageFilter.GaussianBlur(radius=3))


def camera_points() -> list[tuple[float, float, float, float]]:
    return [
        (0.0, 1120.0, 1050.0, 1.24),
        (3.0, 1710.0, 980.0, 1.15),
        (6.0, 2460.0, 940.0, 1.07),
        (8.8, 3150.0, 900.0, 0.98),
        (12.0, 3500.0, 770.0, 0.86),
    ]


def camera_state(time_value: float) -> tuple[float, float, float]:
    points = camera_points()
    for index in range(len(points) - 1):
        t0, x0, y0, z0 = points[index]
        t1, x1, y1, z1 = points[index + 1]
        if time_value <= t1:
            alpha = smoothstep(t0, t1, time_value)
            return (
                mix(x0, x1, alpha),
                mix(y0, y1, alpha) + math.sin(time_value * 0.8) * 10.0,
                mix(z0, z1, alpha),
            )
    _, x, y, z = points[-1]
    return x, y, z


def crop_layer(layer: Image.Image, camera_x: float, camera_y: float, zoom: float, parallax_x: float, parallax_y: float) -> Image.Image:
    view_w = VIDEO_WIDTH / zoom
    view_h = VIDEO_HEIGHT / zoom
    center_x = camera_x * parallax_x
    center_y = camera_y * parallax_y
    left = int(clamp(center_x - view_w * 0.5, 0.0, layer.width - view_w))
    top = int(clamp(center_y - view_h * 0.5, 0.0, layer.height - view_h))
    crop = layer.crop((left, top, int(left + view_w), int(top + view_h)))
    return crop.resize((VIDEO_WIDTH, VIDEO_HEIGHT), Image.Resampling.LANCZOS)


def world_to_screen(world_x: float, world_y: float, camera_x: float, camera_y: float, zoom: float, parallax_x: float, parallax_y: float) -> tuple[float, float]:
    view_w = VIDEO_WIDTH / zoom
    view_h = VIDEO_HEIGHT / zoom
    left = camera_x * parallax_x - view_w * 0.5
    top = camera_y * parallax_y - view_h * 0.5
    screen_x = (world_x - left) * zoom
    screen_y = (world_y - top) * zoom
    return screen_x, screen_y


def render_creatures(time_value: float, camera_x: float, camera_y: float, zoom: float) -> Image.Image:
    overlay = Image.new("RGBA", (VIDEO_WIDTH, VIDEO_HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)

    fox_x, fox_y = world_to_screen(1450 + time_value * 36.0, 1426 + math.sin(time_value * 12.0) * 6.0, camera_x, camera_y, zoom, 0.95, 0.95)
    if -160 < fox_x < VIDEO_WIDTH + 160:
        draw_fox(draw, int(fox_x), int(fox_y), max(0.52, 0.92 * zoom))

    wolf_a = world_to_screen(2190, 1342 + math.sin(time_value * 2.0) * 5.0, camera_x, camera_y, zoom, 0.85, 0.88)
    wolf_b = world_to_screen(2310, 1360 + math.cos(time_value * 1.6) * 4.0, camera_x, camera_y, zoom, 0.85, 0.88)
    if -140 < wolf_a[0] < VIDEO_WIDTH + 140:
        draw_wolf(draw, int(wolf_a[0]), int(wolf_a[1]), max(0.48, 0.88 * zoom))
    if -140 < wolf_b[0] < VIDEO_WIDTH + 140:
        draw_wolf(draw, int(wolf_b[0]), int(wolf_b[1]), max(0.42, 0.72 * zoom))

    kobolds = [
        (2770, 1288, 0.86, 1),
        (2880, 1302, 0.78, -1),
        (3020, 1284, 0.90, 1),
    ]
    for base_x, base_y, scale, facing in kobolds:
        sx, sy = world_to_screen(base_x, base_y + math.sin(time_value * (2.2 + scale)) * 4.0, camera_x, camera_y, zoom, 0.85, 0.88)
        if -120 < sx < VIDEO_WIDTH + 120:
            draw_kobold(draw, int(sx), int(sy), max(0.38, scale * zoom), facing)
    return overlay


def render_smoke(time_value: float, camera_x: float, camera_y: float, zoom: float) -> Image.Image:
    overlay = Image.new("RGBA", (VIDEO_WIDTH, VIDEO_HEIGHT), (0, 0, 0, 0))
    draw = ImageDraw.Draw(overlay)
    world_origin_x = 2820
    world_origin_y = 1234
    for puff in range(7):
        local_time = (time_value * 0.85 + puff * 0.28) % 3.8
        rise = local_time * 72.0
        drift = math.sin(local_time * 1.9 + puff) * 18.0
        sx, sy = world_to_screen(world_origin_x + drift, world_origin_y - rise, camera_x, camera_y, zoom, 0.85, 0.88)
        radius = (18 + puff * 4 + local_time * 5) * zoom
        alpha = max(0, int((80 - local_time * 18) * clamp(zoom)))
        if alpha <= 0:
            continue
        draw.ellipse((sx - radius, sy - radius, sx + radius, sy + radius), fill=(255, 246, 233, alpha))
    return overlay.filter(ImageFilter.GaussianBlur(radius=6))


def render_title(time_value: float, logo: Image.Image) -> Image.Image:
    overlay = Image.new("RGBA", (VIDEO_WIDTH, VIDEO_HEIGHT), (0, 0, 0, 0))
    reveal = smoothstep(9.2, 11.2, time_value)
    if reveal <= 0.0:
        return overlay

    scale = mix(1.10, 1.0, reveal)
    alpha = int(255 * reveal)
    target_width = int(logo.width * scale)
    target_height = int(logo.height * scale)
    logo_resized = logo.resize((target_width, target_height), Image.Resampling.LANCZOS).copy()
    logo_resized.putalpha(logo_resized.getchannel("A").point(lambda value: int(value * alpha / 255)))

    x = (VIDEO_WIDTH - target_width) // 2
    y = int(mix(420, 300, reveal))
    overlay.alpha_composite(logo_resized, (x, y))

    draw = ImageDraw.Draw(overlay)
    subtitle_alpha = int(190 * smoothstep(10.0, 11.6, time_value))
    if subtitle_alpha > 0:
        font = ImageFont.truetype(str(FONT_PATH), 34)
        subtitle = "A frontier of sunlight, secrets, and skybound adventure"
        box = draw.textbbox((0, 0), subtitle, font=font)
        draw.text(
            ((VIDEO_WIDTH - (box[2] - box[0])) // 2, y + target_height - 8),
            subtitle,
            font=font,
            fill=(250, 244, 222, subtitle_alpha),
            stroke_fill=(92, 58, 28, int(subtitle_alpha * 0.55)),
            stroke_width=2,
        )
    return overlay


def render_frame(
    frame_index: int,
    sky: Image.Image,
    far_forest: Image.Image,
    midground: Image.Image,
    foreground: Image.Image,
    rays: Image.Image,
    particles: Image.Image,
    logo: Image.Image,
) -> Image.Image:
    time_value = frame_index / FPS
    camera_x, camera_y, zoom = camera_state(time_value)

    frame = crop_layer(sky, camera_x, camera_y, zoom, 0.22, 0.18)
    frame.alpha_composite(crop_layer(far_forest, camera_x, camera_y, zoom, 0.56, 0.62))
    frame.alpha_composite(crop_layer(midground, camera_x, camera_y, zoom, 0.82, 0.85))
    frame.alpha_composite(render_smoke(time_value, camera_x, camera_y, zoom))
    frame.alpha_composite(render_creatures(time_value, camera_x, camera_y, zoom))
    frame.alpha_composite(crop_layer(rays, camera_x, camera_y, zoom, 0.34, 0.26))
    particle_shifted = crop_layer(particles, camera_x + time_value * 22.0, camera_y, zoom, 0.48, 0.44)
    frame.alpha_composite(particle_shifted)
    frame.alpha_composite(crop_layer(foreground, camera_x, camera_y, zoom, 1.02, 1.0))
    frame.alpha_composite(render_title(time_value, logo))
    add_texture(frame, 10)
    return frame


def note_frequency(name: str) -> float:
    notes = {
        "C": -9,
        "C#": -8,
        "D": -7,
        "D#": -6,
        "E": -5,
        "F": -4,
        "F#": -3,
        "G": -2,
        "G#": -1,
        "A": 0,
        "A#": 1,
        "B": 2,
    }
    if len(name) == 2:
        pitch = name[0]
        octave = int(name[1])
        accidental = ""
    else:
        pitch = name[0]
        accidental = name[1]
        octave = int(name[2])
    semitone = notes[pitch + accidental] + (octave - 4) * 12
    return 440.0 * (2.0 ** (semitone / 12.0))


def add_pluck(buffer_left: array, buffer_right: array, start: float, length: float, frequency: float, amplitude: float, pan: float) -> None:
    start_index = int(start * SAMPLE_RATE)
    end_index = min(len(buffer_left), int((start + length) * SAMPLE_RATE))
    for index in range(start_index, end_index):
        t = (index - start_index) / SAMPLE_RATE
        env = math.exp(-4.2 * t / max(length, 0.001))
        env *= min(1.0, t * 20.0)
        sample = (
            math.sin(math.tau * frequency * t)
            + 0.35 * math.sin(math.tau * frequency * 2.0 * t + 0.3)
            + 0.12 * math.sin(math.tau * frequency * 3.0 * t + 1.2)
        ) * amplitude * env
        left_gain = 0.5 - pan * 0.22
        right_gain = 0.5 + pan * 0.22
        buffer_left[index] += sample * left_gain
        buffer_right[index] += sample * right_gain


def add_flute(buffer_left: array, buffer_right: array, start: float, length: float, frequency: float, amplitude: float, pan: float) -> None:
    start_index = int(start * SAMPLE_RATE)
    end_index = min(len(buffer_left), int((start + length) * SAMPLE_RATE))
    attack = min(0.12, length * 0.25)
    release = min(0.28, length * 0.3)
    for index in range(start_index, end_index):
        t = (index - start_index) / SAMPLE_RATE
        remain = length - t
        env = min(1.0, t / max(attack, 0.001)) * min(1.0, remain / max(release, 0.001))
        vibrato = math.sin(math.tau * 5.2 * t) * 0.006
        sample = (
            math.sin(math.tau * frequency * (1.0 + vibrato) * t)
            + 0.16 * math.sin(math.tau * frequency * 2.0 * t + 0.6)
            + 0.08 * math.sin(math.tau * frequency * 3.0 * t + 1.2)
        ) * amplitude * env
        left_gain = 0.5 - pan * 0.2
        right_gain = 0.5 + pan * 0.2
        buffer_left[index] += sample * left_gain
        buffer_right[index] += sample * right_gain


def add_pad(buffer_left: array, buffer_right: array, start: float, length: float, frequency: float, amplitude: float, pan: float) -> None:
    start_index = int(start * SAMPLE_RATE)
    end_index = min(len(buffer_left), int((start + length) * SAMPLE_RATE))
    for index in range(start_index, end_index):
        t = (index - start_index) / SAMPLE_RATE
        env = min(1.0, t * 1.4) * min(1.0, (length - t) * 1.2)
        sample = (
            math.sin(math.tau * frequency * t)
            + 0.45 * math.sin(math.tau * frequency * 0.5 * t + 0.7)
            + 0.28 * math.sin(math.tau * frequency * 1.5 * t + 0.2)
        ) * amplitude * env
        left_gain = 0.5 - pan * 0.12
        right_gain = 0.5 + pan * 0.12
        buffer_left[index] += sample * left_gain
        buffer_right[index] += sample * right_gain


def build_audio() -> None:
    total_samples = int(DURATION * SAMPLE_RATE)
    left = array("f", [0.0]) * total_samples
    right = array("f", [0.0]) * total_samples
    beat = 60.0 / 96.0

    progression = [
        (0, ["D3", "A3", "D4"]),
        (4, ["G3", "D4", "G4"]),
        (8, ["B2", "F#3", "B3"]),
        (12, ["A2", "E3", "A3"]),
        (16, ["G3", "D4", "A4"]),
    ]
    for beat_index, chord in progression:
        start = beat_index * beat
        length = 4 * beat
        pans = (-0.2, 0.0, 0.18)
        for note, pan in zip(chord, pans):
            add_pad(left, right, start, length + 0.3, note_frequency(note), 0.08, pan)
        arpeggio = [chord[0], chord[1], chord[2], chord[1]]
        for step, note in enumerate(arpeggio):
            add_pluck(left, right, start + step * beat, beat * 0.95, note_frequency(note), 0.20, -0.1 + step * 0.06)
            add_pluck(left, right, start + step * beat + beat * 0.5, beat * 0.8, note_frequency(chord[2]), 0.10, 0.15)

    melody = [
        (0.5, 1.2, "A4"), (1.7, 0.6, "B4"), (2.4, 0.8, "D5"),
        (3.3, 0.8, "E5"), (4.2, 0.9, "D5"), (5.2, 0.8, "B4"),
        (6.2, 0.9, "A4"), (7.2, 0.9, "F#4"), (8.2, 1.0, "G4"),
        (9.4, 0.9, "A4"), (10.4, 1.1, "D5"),
    ]
    for start, length, note in melody:
        add_flute(left, right, start, length, note_frequency(note), 0.16, 0.14)

    bass = [("D2", 0), ("G2", 4), ("B1", 8), ("A1", 12), ("G2", 16)]
    for note, beat_index in bass:
        add_pluck(left, right, beat_index * beat, beat * 2.0, note_frequency(note), 0.18, -0.25)

    with wave.open(str(AUDIO_PATH), "wb") as wav_file:
        wav_file.setnchannels(2)
        wav_file.setsampwidth(2)
        wav_file.setframerate(SAMPLE_RATE)
        pcm = array("h")
        for sample_left, sample_right in zip(left, right):
            clamped_left = max(-0.98, min(0.98, sample_left))
            clamped_right = max(-0.98, min(0.98, sample_right))
            pcm.append(int(clamped_left * 32767))
            pcm.append(int(clamped_right * 32767))
        wav_file.writeframes(pcm.tobytes())


def render_all_frames() -> None:
    if FRAMES_DIR.exists():
        shutil.rmtree(FRAMES_DIR)
    FRAMES_DIR.mkdir(parents=True, exist_ok=True)

    logo = Image.open(SLATE_DIR / "TitleLogo.png").convert("RGBA")
    sky = build_sky_layer()
    far_forest = build_far_forest_layer()
    midground = build_midground_layer()
    foreground = build_foreground_layer()
    rays = build_rays_layer()
    particles = build_particle_layer()

    frame_count = int(DURATION * FPS)
    for frame_index in range(frame_count):
        frame = render_frame(frame_index, sky, far_forest, midground, foreground, rays, particles, logo)
        frame.save(FRAMES_DIR / f"frame_{frame_index:04d}.png")

    menu_frame = render_frame(int(6.2 * FPS), sky, far_forest, midground, foreground, rays, particles, logo)
    menu_frame.save(MENU_FRAME_PATH)


def encode_video() -> None:
    command = [
        "ffmpeg",
        "-y",
        "-framerate",
        str(FPS),
        "-i",
        str(FRAMES_DIR / "frame_%04d.png"),
        "-i",
        str(AUDIO_PATH),
        "-c:v",
        "libx264",
        "-pix_fmt",
        "yuv420p",
        "-profile:v",
        "high",
        "-movflags",
        "+faststart",
        "-c:a",
        "aac",
        "-b:a",
        "192k",
        str(OUTPUT_VIDEO),
    ]
    subprocess.run(command, check=True)


def export_menu_frame() -> None:
    frame = Image.open(MENU_FRAME_PATH).convert("RGBA")
    frame.save(SLATE_DIR / "MenuSky.png")
    transparent = Image.new("RGBA", (VIDEO_WIDTH, VIDEO_HEIGHT), (0, 0, 0, 0))
    transparent.save(SLATE_DIR / "MenuMidground.png")
    mist = Image.new("RGBA", (VIDEO_WIDTH, VIDEO_HEIGHT), (0, 0, 0, 0))
    mist_draw = ImageDraw.Draw(mist)
    mist_draw.rounded_rectangle((0, 0, VIDEO_WIDTH, VIDEO_HEIGHT), radius=0, fill=(255, 255, 255, 8))
    mist.alpha_composite(Image.open(SLATE_DIR / "MenuMist.png").convert("RGBA"))
    mist.save(SLATE_DIR / "MenuMist.png")
    Image.new("RGBA", (VIDEO_WIDTH, VIDEO_HEIGHT), (0, 0, 0, 0)).save(SLATE_DIR / "MenuForeground.png")
    frame.save(SLATE_DIR / "MenuBackground.png")


def main() -> None:
    ensure_dirs()
    render_all_frames()
    build_audio()
    encode_video()
    export_menu_frame()


if __name__ == "__main__":
    main()
