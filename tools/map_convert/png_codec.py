"""Minimal stdlib-only reader/writer for 8-bit palette (indexed color) PNG files.

Only the subset of the PNG format needed by this tool is supported:
- color type 3 (palette), bit depth 8
- no interlacing
- optional single tRNS chunk describing per-palette-entry alpha values

This avoids adding a third-party dependency (e.g. Pillow) to the conversion tool.
"""

from __future__ import annotations

import struct
import zlib
from dataclasses import dataclass
from pathlib import Path

_PNG_SIGNATURE = b"\x89PNG\r\n\x1a\n"

_COLOR_TYPE_PALETTE = 3


@dataclass
class IndexedImage:
    width: int
    height: int
    palette: list[tuple[int, int, int]]
    transparent_index: int | None
    pixels: bytearray  # 1 byte per pixel, row-major, length == width * height

    def get_pixel(self, x: int, y: int) -> int:
        return self.pixels[y * self.width + x]

    def set_pixel(self, x: int, y: int, value: int) -> None:
        self.pixels[y * self.width + x] = value


def _iter_chunks(data: bytes):
    offset = len(_PNG_SIGNATURE)
    while offset < len(data):
        (length,) = struct.unpack(">I", data[offset:offset + 4])
        chunk_type = data[offset + 4:offset + 8].decode("ascii")
        chunk_data = data[offset + 8:offset + 8 + length]
        offset += 8 + length + 4  # length + type + data + crc
        yield chunk_type, chunk_data


def _paeth_predictor(a: int, b: int, c: int) -> int:
    p = a + b - c
    pa = abs(p - a)
    pb = abs(p - b)
    pc = abs(p - c)
    if pa <= pb and pa <= pc:
        return a
    if pb <= pc:
        return b
    return c


def _unfilter(raw: bytes, width: int, height: int) -> bytearray:
    """Reverse PNG per-scanline filtering for a bit-depth-8, 1-channel-per-pixel image."""
    stride = width  # 1 byte per pixel (bpp = 1)
    out = bytearray(width * height)
    prev_row = bytearray(stride)
    pos = 0
    for y in range(height):
        filter_type = raw[pos]
        pos += 1
        row = bytearray(raw[pos:pos + stride])
        pos += stride

        if filter_type == 0:
            pass
        elif filter_type == 1:  # Sub
            for x in range(stride):
                a = row[x - 1] if x > 0 else 0
                row[x] = (row[x] + a) & 0xFF
        elif filter_type == 2:  # Up
            for x in range(stride):
                b = prev_row[x]
                row[x] = (row[x] + b) & 0xFF
        elif filter_type == 3:  # Average
            for x in range(stride):
                a = row[x - 1] if x > 0 else 0
                b = prev_row[x]
                row[x] = (row[x] + ((a + b) // 2)) & 0xFF
        elif filter_type == 4:  # Paeth
            for x in range(stride):
                a = row[x - 1] if x > 0 else 0
                b = prev_row[x]
                c = prev_row[x - 1] if x > 0 else 0
                row[x] = (row[x] + _paeth_predictor(a, b, c)) & 0xFF
        else:
            raise ValueError(f"Unsupported PNG filter type {filter_type}")

        out[y * stride:(y + 1) * stride] = row
        prev_row = row

    return out


def decode(path: Path) -> IndexedImage:
    path = Path(path)
    data = path.read_bytes()

    if data[:8] != _PNG_SIGNATURE:
        raise ValueError(f"{path}: not a PNG file")

    width = height = None
    bit_depth = color_type = None
    palette: list[tuple[int, int, int]] = []
    transparent_index: int | None = None
    idat_chunks: list[bytes] = []

    for chunk_type, chunk_data in _iter_chunks(data):
        if chunk_type == "IHDR":
            width, height, bit_depth, color_type, compression, filter_method, interlace = struct.unpack(
                ">IIBBBBB", chunk_data
            )
            if compression != 0 or filter_method != 0:
                raise ValueError(f"{path}: unsupported compression/filter method in IHDR")
            if interlace != 0:
                raise ValueError(f"{path}: interlaced PNGs are not supported")
        elif chunk_type == "PLTE":
            if len(chunk_data) % 3 != 0:
                raise ValueError(f"{path}: malformed PLTE chunk")
            palette = [
                (chunk_data[i], chunk_data[i + 1], chunk_data[i + 2])
                for i in range(0, len(chunk_data), 3)
            ]
        elif chunk_type == "tRNS":
            for idx, alpha in enumerate(chunk_data):
                if alpha == 0:
                    transparent_index = idx
                    break
        elif chunk_type == "IDAT":
            idat_chunks.append(chunk_data)
        elif chunk_type == "IEND":
            break

    if width is None or height is None:
        raise ValueError(f"{path}: missing IHDR chunk")
    if color_type != _COLOR_TYPE_PALETTE or bit_depth != 8:
        raise ValueError(
            f"{path}: only 8-bit palette PNGs are supported (color_type={color_type}, bit_depth={bit_depth})"
        )
    if not palette:
        raise ValueError(f"{path}: palette PNG without a PLTE chunk")

    raw = zlib.decompress(b"".join(idat_chunks))
    pixels = _unfilter(raw, width, height)

    return IndexedImage(
        width=width,
        height=height,
        palette=palette,
        transparent_index=transparent_index,
        pixels=pixels,
    )


def _chunk(chunk_type: str, chunk_data: bytes) -> bytes:
    type_bytes = chunk_type.encode("ascii")
    crc = zlib.crc32(type_bytes + chunk_data) & 0xFFFFFFFF
    return struct.pack(">I", len(chunk_data)) + type_bytes + chunk_data + struct.pack(">I", crc)


def encode(image: IndexedImage, path: Path) -> None:
    path = Path(path)

    ihdr = struct.pack(">IIBBBBB", image.width, image.height, 8, _COLOR_TYPE_PALETTE, 0, 0, 0)

    plte = b"".join(struct.pack(">BBB", r, g, b) for r, g, b in image.palette)

    raw = bytearray()
    for y in range(image.height):
        raw.append(0)  # filter type 0 (None) for every scanline
        raw.extend(image.pixels[y * image.width:(y + 1) * image.width])
    idat = zlib.compress(bytes(raw), level=9)

    chunks = [_chunk("IHDR", ihdr), _chunk("PLTE", plte)]

    if image.transparent_index is not None:
        trns = bytes(
            0 if i == image.transparent_index else 255 for i in range(len(image.palette))
        )
        chunks.append(_chunk("tRNS", trns))

    chunks.append(_chunk("IDAT", idat))
    chunks.append(_chunk("IEND", b""))

    path.write_bytes(_PNG_SIGNATURE + b"".join(chunks))
