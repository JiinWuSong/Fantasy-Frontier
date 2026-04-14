#!/usr/bin/env python3
import argparse
import math
import os
import random
import struct
import zlib


def clamp01(value: float) -> float:
    return 0.0 if value < 0.0 else 1.0 if value > 1.0 else value


def lerp(a: float, b: float, t: float) -> float:
    return a + (b - a) * t


def smoothstep(edge0: float, edge1: float, value: float) -> float:
    if edge0 == edge1:
        return 0.0
    t = clamp01((value - edge0) / (edge1 - edge0))
    return t * t * (3.0 - 2.0 * t)


def mix_color(a, b, t: float):
    return (
        int(lerp(a[0], b[0], t)),
        int(lerp(a[1], b[1], t)),
        int(lerp(a[2], b[2], t)),
    )


def write_png(path: str, width: int, height: int, pixels: bytearray) -> None:
    def chunk(chunk_type: bytes, data: bytes) -> bytes:
        return (
            struct.pack(">I", len(data))
            + chunk_type
            + data
            + struct.pack(">I", zlib.crc32(chunk_type + data) & 0xFFFFFFFF)
        )

    raw = bytearray()
    stride = width * 3
    for y in range(height):
        raw.append(0)
        start = y * stride
        raw.extend(pixels[start : start + stride])

    png = bytearray()
    png.extend(b"\x89PNG\r\n\x1a\n")
    png.extend(
        chunk(
            b"IHDR",
            struct.pack(">IIBBBBB", width, height, 8, 2, 0, 0, 0),
        )
    )
    png.extend(chunk(b"IDAT", zlib.compress(bytes(raw), level=9)))
    png.extend(chunk(b"IEND", b""))

    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, "wb") as handle:
        handle.write(png)


class Canvas:
    def __init__(self, width: int, height: int):
        self.width = width
        self.height = height
        self.pixels = bytearray(width * height * 3)

    def blend_pixel(self, x: int, y: int, color, alpha: float) -> None:
        if x < 0 or y < 0 or x >= self.width or y >= self.height:
            return
        alpha = clamp01(alpha)
        index = (y * self.width + x) * 3
        inv = 1.0 - alpha
        self.pixels[index] = int(self.pixels[index] * inv + color[0] * alpha)
        self.pixels[index + 1] = int(self.pixels[index + 1] * inv + color[1] * alpha)
        self.pixels[index + 2] = int(self.pixels[index + 2] * inv + color[2] * alpha)

    def fill_gradient(self) -> None:
        top = (6, 11, 28)
        upper_mid = (10, 30, 62)
        horizon = (18, 74, 88)
        lower = (4, 9, 16)

        for y in range(self.height):
            ny = y / (self.height - 1)
            if ny < 0.58:
                row = mix_color(top, upper_mid, smoothstep(0.0, 0.58, ny))
            else:
                row = mix_color(horizon, lower, smoothstep(0.58, 1.0, ny))

            for x in range(self.width):
                nx = x / (self.width - 1)
                lateral_glow = 0.08 * math.exp(-((nx - 0.52) ** 2) / 0.07)
                cold_band = 0.22 * math.exp(-((ny - (0.20 + 0.05 * math.sin(nx * 8.0))) ** 2) / 0.0030)
                warm_band = 0.16 * math.exp(-((ny - (0.34 + 0.04 * math.sin(nx * 5.3 + 0.8))) ** 2) / 0.0045)
                horizon_glow = 0.22 * math.exp(-((ny - 0.60) ** 2) / 0.0065)

                r = row[0] + int(10 * lateral_glow + 18 * warm_band + 10 * horizon_glow)
                g = row[1] + int(30 * cold_band + 14 * warm_band + 18 * horizon_glow)
                b = row[2] + int(42 * cold_band + 8 * warm_band)

                index = (y * self.width + x) * 3
                self.pixels[index] = min(255, r)
                self.pixels[index + 1] = min(255, g)
                self.pixels[index + 2] = min(255, b)

    def draw_glow(self, cx: float, cy: float, radius: float, color, strength: float) -> None:
        x0 = max(0, int(cx - radius))
        x1 = min(self.width - 1, int(cx + radius))
        y0 = max(0, int(cy - radius))
        y1 = min(self.height - 1, int(cy + radius))

        for y in range(y0, y1 + 1):
            for x in range(x0, x1 + 1):
                distance = math.hypot(x - cx, y - cy)
                if distance > radius:
                    continue
                alpha = strength * math.exp(-((distance / radius) ** 2) * 4.0)
                self.blend_pixel(x, y, color, alpha)

    def draw_moon(self) -> None:
        cx = self.width * 0.79
        cy = self.height * 0.19
        self.draw_glow(cx, cy, self.height * 0.16, (210, 235, 255), 0.26)
        self.draw_glow(cx, cy, self.height * 0.095, (245, 248, 255), 0.42)
        self.draw_glow(cx, cy, self.height * 0.055, (255, 250, 238), 0.70)

    def draw_stars(self, rng: random.Random) -> None:
        star_count = max(420, self.width * self.height // 5200)
        for _ in range(star_count):
            x = rng.randrange(0, self.width)
            y = rng.randrange(0, int(self.height * 0.58))
            radius = 0.7 + rng.random() * 2.4
            brightness = 0.28 + rng.random() * 0.65
            color = (
                220 + rng.randrange(0, 30),
                225 + rng.randrange(0, 25),
                235 + rng.randrange(0, 20),
            )
            self.draw_glow(x, y, radius * 2.8, color, brightness * 0.24)
            self.draw_glow(x, y, radius, color, brightness * 0.92)

            if rng.random() < 0.10:
                cross = int(radius * 3.0) + 2
                for offset in range(-cross, cross + 1):
                    falloff = max(0.0, 1.0 - abs(offset) / max(1.0, cross))
                    self.blend_pixel(x + offset, y, color, brightness * 0.35 * falloff)
                    self.blend_pixel(x, y + offset, color, brightness * 0.35 * falloff)

    def draw_motes(self, rng: random.Random) -> None:
        for _ in range(48):
            x = rng.uniform(self.width * 0.12, self.width * 0.88)
            y = rng.uniform(self.height * 0.42, self.height * 0.78)
            radius = rng.uniform(1.4, 3.8)
            color = (
                214 + rng.randrange(0, 24),
                176 + rng.randrange(0, 24),
                92 + rng.randrange(0, 24),
            )
            self.draw_glow(x, y, radius * 4.0, color, 0.08)
            self.draw_glow(x, y, radius, color, 0.62)

    def ridge(self, rng: random.Random, base_y: float, amplitude: float, points: int):
        control_count = max(4, points)
        step = (self.width - 1) / (control_count - 1)
        control = []
        for index in range(control_count):
            x = int(round(index * step))
            wave = math.sin(index * 0.85 + rng.random() * 0.2)
            y = base_y + rng.uniform(-amplitude, amplitude) + wave * amplitude * 0.35
            control.append((x, y))

        ridge = [0.0] * self.width
        for left_index in range(control_count - 1):
            x0, y0 = control[left_index]
            x1, y1 = control[left_index + 1]
            span = max(1, x1 - x0)
            for x in range(x0, x1 + 1):
                t = (x - x0) / span
                ridge[x] = lerp(y0, y1, smoothstep(0.0, 1.0, t))
        return ridge

    def fill_under_ridge(self, ridge, color, alpha: float) -> None:
        for x in range(self.width):
            start_y = int(ridge[x])
            for y in range(max(0, start_y), self.height):
                fog = 0.0
                if y < self.height * 0.76:
                    fog = 0.12 * math.exp(-((y - self.height * 0.62) ** 2) / (self.height * 14.0))
                self.blend_pixel(x, y, color, alpha + fog)

    def add_mist(self) -> None:
        for y in range(int(self.height * 0.46), int(self.height * 0.76)):
            ny = y / self.height
            band_strength = (
                0.11 * math.exp(-((ny - 0.56) ** 2) / 0.0024)
                + 0.09 * math.exp(-((ny - 0.63) ** 2) / 0.0018)
            )
            if band_strength <= 0.001:
                continue

            for x in range(self.width):
                nx = x / self.width
                wave = (
                    0.55
                    + 0.25 * math.sin(nx * 9.0 + ny * 14.0)
                    + 0.20 * math.sin(nx * 17.0 - ny * 11.0)
                )
                alpha = clamp01(band_strength * wave)
                self.blend_pixel(x, y, (132, 166, 180), alpha)

    def add_vignette(self) -> None:
        cx = self.width * 0.5
        cy = self.height * 0.48
        radius = math.hypot(cx, cy)

        for y in range(self.height):
            for x in range(self.width):
                distance = math.hypot(x - cx, y - cy) / radius
                shadow = smoothstep(0.58, 1.0, distance)
                if shadow <= 0.0:
                    continue
                self.blend_pixel(x, y, (0, 0, 0), shadow * 0.46)


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate a fantasy title-screen background.")
    parser.add_argument("--output", required=True, help="PNG output path")
    parser.add_argument("--width", type=int, default=1920)
    parser.add_argument("--height", type=int, default=1080)
    parser.add_argument("--seed", type=int, default=1404)
    args = parser.parse_args()

    rng = random.Random(args.seed)
    canvas = Canvas(args.width, args.height)
    canvas.fill_gradient()
    canvas.draw_moon()
    canvas.draw_stars(rng)
    canvas.add_mist()

    far_ridge = canvas.ridge(rng, args.height * 0.56, args.height * 0.045, 8)
    mid_ridge = canvas.ridge(rng, args.height * 0.68, args.height * 0.05, 9)
    front_ridge = canvas.ridge(rng, args.height * 0.80, args.height * 0.035, 11)

    canvas.fill_under_ridge(far_ridge, (20, 34, 48), 0.72)
    canvas.fill_under_ridge(mid_ridge, (12, 21, 32), 0.88)
    canvas.fill_under_ridge(front_ridge, (6, 10, 18), 0.96)
    canvas.draw_motes(rng)
    canvas.add_vignette()

    write_png(args.output, args.width, args.height, canvas.pixels)


if __name__ == "__main__":
    main()
