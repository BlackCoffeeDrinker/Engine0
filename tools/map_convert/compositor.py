"""Per-cell layer flattening: reduce the TMX layers to a single tile id per cell,
creating (and deduplicating) composite tiles in the atlas as needed."""

from __future__ import annotations

from dataclasses import dataclass

from atlas import AtlasBuilder
from png_codec import IndexedImage
from tmx_reader import TmxMap
from tsx_reader import AnimationFrame, TsxTileset

# Sentinel used for "no tile here" cells. The engine itself treats a final map cell
# value of 0 as "nothing to draw", so blank cells never need a real atlas tile - they
# just carry this sentinel through flattening/compaction until the writer emits 0.
EMPTY_TILE_ID = -1


@dataclass
class FlattenResult:
    tile_ids: list[list[int]]  # [y][x] -> output local tile id (or EMPTY_TILE_ID)
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


def flatten_above_player(tmx_map: TmxMap, blank_tile_id: int = EMPTY_TILE_ID) -> list[list[int]] | None:
    """Remap the above-player layer's gids to local tile ids directly (no blending needed,
    since it is already a single source layer). Empty cells get `blank_tile_id`
    (`EMPTY_TILE_ID` by default), which the writer later turns into the literal value 0
    the engine treats as "no tile". Returns None if the layer is absent or every cell in
    it is blank."""
    layer = tmx_map.above_player_layer
    if layer is None:
        return None

    tile_ids: list[list[int]] = []
    has_data = False
    for y in range(layer.height):
        row: list[int] = []
        for x in range(layer.width):
            gid = layer.gid_at(x, y)
            if gid != 0:
                has_data = True
                row.append(gid - tmx_map.firstgid)
            else:
                row.append(blank_tile_id)
        tile_ids.append(row)

    if not has_data:
        return None

    return tile_ids


def compute_used_tile_ids(
    tile_ids: list[list[int]],
    above_player_tile_ids: list[list[int]] | None,
    animations: dict[int, list[AnimationFrame]],
) -> set[int]:
    """Collect every tile id referenced by the ground/above-player grids, plus any
    animation frame tile ids reachable from an animated tile that is itself used
    (so mid-animation frames aren't dropped as "unused")."""
    used: set[int] = set()
    for row in tile_ids:
        used.update(row)
    if above_player_tile_ids is not None:
        for row in above_player_tile_ids:
            used.update(row)

    used.discard(EMPTY_TILE_ID)

    for tile_id in list(used):
        for frame in animations.get(tile_id, []):
            used.add(frame.tile_id)

    return used


def flatten(tmx_map: TmxMap, tileset: TsxTileset, atlas: AtlasBuilder) -> FlattenResult:
    # The engine ignores a final cell value of 0 ("no tile"), so blank cells don't need a
    # real atlas tile - they carry the EMPTY_TILE_ID sentinel through instead.
    blank_tile_id = EMPTY_TILE_ID

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
