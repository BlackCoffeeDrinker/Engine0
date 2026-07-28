"""Per-cell layer flattening: reduce the TMX layers to a single tile id per cell,
creating (and deduplicating) composite tiles in the atlas as needed."""

from __future__ import annotations

from dataclasses import dataclass

from atlas import AtlasBuilder
from png_codec import IndexedImage
from tmx_reader import TmxMap
from tsx_reader import AnimationFrame, TsxTileset


@dataclass
class FlattenResult:
    tile_ids: list[list[int]]  # [y][x] -> output local tile id
    animations: dict[int, list[AnimationFrame]]  # output tile id -> animation frames (direct-mapped tiles only)
    blank_tile_id: int


def _blend(bottom: IndexedImage, top: IndexedImage) -> IndexedImage:
    """Draw `top` over `bottom`, treating the palette's transparent index as see-through."""
    result_pixels = bytearray(bottom.pixels)
    transparent = top.transparent_index
    for i, value in enumerate(top.pixels):
        if transparent is None or value != transparent:
            result_pixels[i] = value

    return IndexedImage(
        width=bottom.width,
        height=bottom.height,
        palette=bottom.palette,
        transparent_index=bottom.transparent_index,
        pixels=result_pixels,
    )


def flatten(tmx_map: TmxMap, tileset: TsxTileset, atlas: AtlasBuilder) -> FlattenResult:
    blank_tile_id = atlas.append_tile(atlas.make_blank_tile())

    composite_cache: dict[bytes, int] = {}
    animations: dict[int, list[AnimationFrame]] = {}

    tile_ids: list[list[int]] = []
    for y in range(tmx_map.height):
        row: list[int] = []
        for x in range(tmx_map.width):
            local_ids = []
            for layer in tmx_map.layers:
                gid = layer.gid_at(x, y)
                if gid != 0:
                    local_ids.append(gid - tmx_map.firstgid)

            if not local_ids:
                row.append(blank_tile_id)
                continue

            if len(local_ids) == 1:
                tile_id = local_ids[0]
                if tile_id in tileset.animations:
                    animations[tile_id] = tileset.animations[tile_id]
                row.append(tile_id)
                continue

            composite = atlas.get_tile(local_ids[0])
            for local_id in local_ids[1:]:
                composite = _blend(composite, atlas.get_tile(local_id))

            composite_hash = bytes(composite.pixels)
            existing_id = composite_cache.get(composite_hash)
            if existing_id is None:
                existing_id = atlas.append_tile(composite)
                composite_cache[composite_hash] = existing_id

            row.append(existing_id)

        tile_ids.append(row)

    return FlattenResult(tile_ids=tile_ids, animations=animations, blank_tile_id=blank_tile_id)
