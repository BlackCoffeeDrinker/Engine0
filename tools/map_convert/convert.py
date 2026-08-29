#!/usr/bin/env python3
"""CLI entry point: convert a Tiled `.tmx` map into the engine's `wh1.ini` /
`basicset.ini` / `tileset.png` resource files.

Usage:
    python tools/map_convert/convert.py [input.tmx] [--out-dir example/res] \
        [--map-name wh1] [--atlas-name tileset.png] [--palette example/res/pal.ini]
"""

from __future__ import annotations

import argparse
import os.path
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import png_codec
import writer
from atlas import AtlasBuilder, compact
from compositor import EMPTY_TILE_ID, compute_used_tile_ids, flatten, flatten_above_player
from palette import load_palette_ini, remap_to_palette
from tmx_reader import parse_tmx
from tsx_reader import AnimationFrame, TsxTile, parse_tsx

_DEFAULT_TMX = "example/source/maps/world-1-1.tmx"


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", nargs="?", default=_DEFAULT_TMX, help="Path to the source .tmx map")
    parser.add_argument("--out-dir", default="example/res", help="Directory to write the output files to")
    parser.add_argument("--map-name", default="wh1", help="Base name for the map .ini/.dat files")
    parser.add_argument("--atlas-name", default="tileset.png", help="File name for the output atlas PNG")
    parser.add_argument(
        "--tileset-name",
        default=None,
        help="Base name/resource key for the output tileset resource .ini "
             "(default: '<map-name>tileset')",
    )
    parser.add_argument(
        "--bitmap-resource-key",
        default=None,
        help="Resource key for the atlas bitmap referenced by the tileset .ini "
             "(default: '--atlas-name' without its extension)",
    )
    parser.add_argument(
        "--emit-dat",
        action="store_true",
        help="Write separate .dat file(s) for the [set]/[setaboveplayer] tile data instead "
             "of inline `data =` rows",
    )
    parser.add_argument(
        "--palette",
        default=None,
        help="Path to a pal.ini-style palette file to remap the output atlas to (see example/res/pal.ini)",
    )
    return parser.parse_args(argv)


def convert(
        tmx_path: Path,
        out_dir: Path,
        map_name: str,
        atlas_name: str,
        tileset_name: str | None = None,
        bitmap_resource_key: str | None = None,
        emit_dat: bool = False,
        palette_path: Path | None = None,
) -> None:
    if tileset_name is None:
        tileset_name = f"{map_name}tileset"
    if bitmap_resource_key is None:
        bitmap_resource_key = Path(atlas_name).stem

    tmx_map = parse_tmx(tmx_path)
    tileset = parse_tsx(tmx_map.tsx_path)
    source_image = png_codec.decode(tileset.image_path)

    atlas = AtlasBuilder(
        source_image,
        tile_width=tileset.tile_width,
        tile_height=tileset.tile_height,
        columns=tileset.columns,
        tile_count=tileset.tile_count,
    )

    result = flatten(tmx_map, tileset, atlas)
    above_player_tile_ids = flatten_above_player(tmx_map, result.blank_tile_id)

    # Drop any tileset tiles that aren't actually referenced anywhere on the map (or by
    # an animation reachable from a referenced tile) - no forced blank tile is kept,
    # since the engine ignores a final cell value of 0 ("no tile") and never draws it.
    used_tile_ids = compute_used_tile_ids(result.tile_ids, above_player_tile_ids, tileset.animations)
    atlas, remap = compact(atlas, used_tile_ids)

    # `starting = 1` reserves the literal cell value 0 for "no tile" (ignored by the
    # engine), so every real tile is shifted up by one: compacted local id 0 -> cell
    # value 1, local id 1 -> cell value 2, and so on.
    starting = 1

    def _to_cell_value(tile_id: int) -> int:
        return 0 if tile_id == EMPTY_TILE_ID else remap[tile_id] + starting

    set_tile_ids = [[_to_cell_value(tile_id) for tile_id in row] for row in result.tile_ids]
    if above_player_tile_ids is not None:
        above_player_tile_ids = [[_to_cell_value(tile_id) for tile_id in row] for row in above_player_tile_ids]

    animations: dict[int, list[AnimationFrame]] = {}
    for old_id, frames in tileset.animations.items():
        if old_id not in remap:
            continue
        animations[remap[old_id] + starting] = [
            AnimationFrame(tile_id=remap[frame.tile_id] + starting, duration_ms=frame.duration_ms)
            for frame in frames
            if frame.tile_id in remap
        ]

    tiles: dict[int, TsxTile] = {}
    for old_id, tile in tileset.tiles.items():
        if old_id not in remap:
            continue
        new_id = remap[old_id] + starting
        tiles[new_id] = TsxTile(tile_id=new_id, class_name=tile.class_name, collision=tile.collision)

    if palette_path is not None:
        new_palette = load_palette_ini(palette_path)
        atlas.image = remap_to_palette(atlas.image, new_palette)

    out_dir.mkdir(parents=True, exist_ok=True)
    ini_path = out_dir / f"{map_name}.ini"
    tileset_ini_path = out_dir / f"{tileset_name}.ini"
    atlas_path = out_dir / atlas_name

    set_resource_key = None
    above_player_resource_key = None

    if emit_dat:
        set_dat_path = out_dir / f"{map_name}-0.dat"
        writer.write_dat(set_dat_path, set_tile_ids)
        set_resource_key = f"{map_name}-0-dat"
        print(f"Wrote {set_dat_path}")

        if above_player_tile_ids is not None:
            above_player_dat_path = out_dir / f"{map_name}-1.dat"
            writer.write_dat(above_player_dat_path, above_player_tile_ids)
            above_player_resource_key = f"{map_name}-1-dat"
            print(f"Wrote {above_player_dat_path}")

    writer.write_ini(
        ini_path,
        width=tmx_map.width,
        height=tmx_map.height,
        set_tile_ids=set_tile_ids,
        above_player_tile_ids=above_player_tile_ids,
        objects=tmx_map.objects,
        tileset_resource_key=tileset_name,
        starting=starting,
        set_resource_key=set_resource_key,
        above_player_resource_key=above_player_resource_key,
    )
    writer.write_tileset_ini(
        tileset_ini_path,
        bitmap_resource_key=bitmap_resource_key,
        tile_width=tileset.tile_width,
        tile_height=tileset.tile_height,
        tiles=tiles,
        animations=animations,
    )
    png_codec.encode(atlas.image, atlas_path)
    
    for obj in tmx_map.objects:
        if obj.sprite_id is None:
            continue
        sprite_path = f"{out_dir}/s{obj.sprite_id}.ini"
        if os.path.exists(sprite_path):
            continue
        # TODO: Write sprite ini
        print(f"{obj.name}: {obj.x}, {obj.y}")

    print(f"Wrote {ini_path}")
    print(f"Wrote {tileset_ini_path}")
    print(f"Wrote {atlas_path} ({atlas.image.width}x{atlas.image.height})")


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    palette_path = Path(args.palette) if args.palette is not None else None
    convert(
        Path(args.input),
        Path(args.out_dir),
        args.map_name,
        args.atlas_name,
        tileset_name=args.tileset_name,
        bitmap_resource_key=args.bitmap_resource_key,
        emit_dat=args.emit_dat,
        palette_path=palette_path,
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
