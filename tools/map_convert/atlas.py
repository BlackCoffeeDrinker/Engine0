"""In-memory tile atlas: crop existing tiles by id, append new composite tiles as
extra rows, and re-encode the whole thing back to an 8-bit palette PNG."""

from __future__ import annotations

from png_codec import IndexedImage


class AtlasBuilder:
    def __init__(self, image: IndexedImage, tile_width: int, tile_height: int, columns: int, tile_count: int):
        self.image = image
        self.tile_width = tile_width
        self.tile_height = tile_height
        self.columns = columns
        self._next_local_id = tile_count

    def get_tile(self, local_id: int) -> IndexedImage:
        """Crop the tile-sized block for an already-existing local tile id."""
        row, col = divmod(local_id, self.columns)
        x0 = col * self.tile_width
        y0 = row * self.tile_height

        pixels = bytearray(self.tile_width * self.tile_height)
        for y in range(self.tile_height):
            src_row_start = (y0 + y) * self.image.width + x0
            pixels[y * self.tile_width:(y + 1) * self.tile_width] = (
                self.image.pixels[src_row_start:src_row_start + self.tile_width]
            )

        return IndexedImage(
            width=self.tile_width,
            height=self.tile_height,
            palette=self.image.palette,
            transparent_index=self.image.transparent_index,
            pixels=pixels,
        )

    def _ensure_row_exists(self, row: int) -> None:
        required_height = (row + 1) * self.tile_height
        if required_height <= self.image.height:
            return

        fill_value = self.image.transparent_index if self.image.transparent_index is not None else 0
        extra_rows = required_height - self.image.height
        self.image.pixels.extend(bytes([fill_value]) * (self.image.width * extra_rows))
        self.image.height = required_height

    def append_tile(self, tile: IndexedImage) -> int:
        """Append a new composite tile image to the atlas, returning its new local id."""
        if tile.width != self.tile_width or tile.height != self.tile_height:
            raise ValueError(
                f"Tile size mismatch: expected {self.tile_width}x{self.tile_height}, got {tile.width}x{tile.height}"
            )

        local_id = self._next_local_id
        self._next_local_id += 1

        row, col = divmod(local_id, self.columns)
        self._ensure_row_exists(row)

        x0 = col * self.tile_width
        y0 = row * self.tile_height
        for y in range(self.tile_height):
            dst_row_start = (y0 + y) * self.image.width + x0
            self.image.pixels[dst_row_start:dst_row_start + self.tile_width] = (
                tile.pixels[y * self.tile_width:(y + 1) * self.tile_width]
            )

        return local_id

    def make_blank_tile(self) -> IndexedImage:
        """Build a fully-transparent tile-sized image, using the atlas' transparent palette index."""
        fill_value = self.image.transparent_index if self.image.transparent_index is not None else 0
        pixels = bytearray([fill_value]) * (self.tile_width * self.tile_height)
        return IndexedImage(
            width=self.tile_width,
            height=self.tile_height,
            palette=self.image.palette,
            transparent_index=self.image.transparent_index,
            pixels=pixels,
        )
