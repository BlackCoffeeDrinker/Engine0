#!/usr/bin/env python3
"""CLI entry point: convert a Tiled `.tmx` map into the engine's `wh1.ini` /
`wh1-0.dat` / `tileset.png` resource files.

Usage:
    python tools/map_convert/convert.py [input.tmx] [--out-dir example/res] \
        [--map-name wh1] [--atlas-name tileset.png]
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))

import png_codec
import writer
from atlas import AtlasBuilder
from compositor import flatten
from tmx_reader import parse_tmx
from tsx_reader import parse_tsx

_DEFAULT_TMX = "example/source/maps/world-1-1.tmx"


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input", nargs="?", default=_DEFAULT_TMX, help="Path to the source .tmx map")
    parser.add_argument("--out-dir", default="example/res", help="Directory to write the output files to")
    parser.add_argument("--map-name", default="wh1", help="Base name for the map .ini/.dat files")
    parser.add_argument("--atlas-name", default="tileset.png", help="File name for the output atlas PNG")
    return parser.parse_args(argv)


def convert(tmx_path: Path, out_dir: Path, map_name: str, atlas_name: str) -> None:
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

    out_dir.mkdir(parents=True, exist_ok=True)
    ini_path = out_dir / f"{map_name}.ini"
    dat_path = out_dir / f"{map_name}-0.dat"
    atlas_path = out_dir / atlas_name

    writer.write_ini(
        ini_path,
        width=tmx_map.width,
        height=tmx_map.height,
        tile_width=tileset.tile_width,
        tile_height=tileset.tile_height,
        animations=result.animations,
    )
    writer.write_dat(dat_path, result.tile_ids)
    png_codec.encode(atlas.image, atlas_path)

    print(f"Wrote {ini_path}")
    print(f"Wrote {dat_path}")
    print(f"Wrote {atlas_path} ({atlas.image.width}x{atlas.image.height})")


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    convert(Path(args.input), Path(args.out_dir), args.map_name, args.atlas_name)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
