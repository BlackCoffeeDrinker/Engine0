"""Load a `pal.ini`-style palette file (see `example/res/pal.ini`) and remap an
`IndexedImage` to use it, matching the engine's `FixedPalette`/`PaletteLoader` format
and nearest-color-matching behaviour (`FixedPalette::findClosestColorIndex`)."""

from __future__ import annotations

import configparser
from pathlib import Path

from png_codec import IndexedImage

RGB = tuple[int, int, int]


def _parse_color(value: str) -> RGB:
    value = value.strip()

    if value.startswith("#"):
        hex_digits = value[1:]
        if len(hex_digits) == 3:
            hex_digits = "".join(c * 2 for c in hex_digits)
        if len(hex_digits) != 6:
            raise ValueError(f"Invalid hex color: {value!r}")
        return (
            int(hex_digits[0:2], 16),
            int(hex_digits[2:4], 16),
            int(hex_digits[4:6], 16),
        )

    parts = value.split()
    if len(parts) != 3:
        raise ValueError(f"Invalid color: {value!r} (expected 'R G B' or '#RRGGBB')")
    r, g, b = (int(part) for part in parts)
    return (r, g, b)


def load_palette_ini(path: Path) -> list[RGB]:
    """Parse a `[palette]`/`[colors]` `.ini` file into an ordered list of RGB tuples."""
    config = configparser.ConfigParser()
    config.read(Path(path))

    num_colors = config.getint("palette", "colors")
    colors: list[RGB | None] = [None] * num_colors

    for key, value in config.items("colors"):
        index = int(key)
        if index >= num_colors:
            raise ValueError(f"{path}: color index {index} is out of range (colors = {num_colors})")
        colors[index] = _parse_color(value)

    for index, color in enumerate(colors):
        if color is None:
            raise ValueError(f"{path}: missing color entry for index {index}")

    return colors  # type: ignore[return-value]


def _closest_color_index(color: RGB, palette: list[RGB]) -> int:
    r, g, b = color
    best_index = 0
    best_distance = None
    for index, (pr, pg, pb) in enumerate(palette):
        distance = (pr - r) ** 2 + (pg - g) ** 2 + (pb - b) ** 2
        if distance == 0:
            return index
        if best_distance is None or distance < best_distance:
            best_distance = distance
            best_index = index
    return best_index


def remap_to_palette(image: IndexedImage, new_palette: list[RGB]) -> IndexedImage:
    """Re-index `image`'s pixels against `new_palette`, mapping each old palette
    entry to its nearest-matching color in the new one."""
    index_map = [_closest_color_index(color, new_palette) for color in image.palette]

    new_transparent_index = (
        index_map[image.transparent_index] if image.transparent_index is not None else None
    )

    new_pixels = bytearray(index_map[value] for value in image.pixels)

    return IndexedImage(
        width=image.width,
        height=image.height,
        palette=list(new_palette),
        transparent_index=new_transparent_index,
        pixels=new_pixels,
    )
