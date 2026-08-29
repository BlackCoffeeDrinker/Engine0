from __future__ import annotations

from pathlib import Path

from tmx_reader import TmxObject
from tsx_reader import AnimationFrame, TsxTile

# Default `[resourcemap]` keys already declared in `example/res/game.ini`.
DEFAULT_TILESET_RESOURCE_KEY = "basicset"


def _format_data_lines(tile_ids: list[list[int]]) -> list[str]:
    return [f"data = {','.join(str(tile_id) for tile_id in row)}," for row in tile_ids]


def write_ini(
        ini_path: Path,
        width: int,
        height: int,
        set_tile_ids: list[list[int]],
        above_player_tile_ids: list[list[int]] | None,
        objects: list[TmxObject],
        tileset_resource_key: str = DEFAULT_TILESET_RESOURCE_KEY,
        starting: int = 1,
        set_resource_key: str | None = None,
        above_player_resource_key: str | None = None,
) -> None:
    lines = [
        "[map]",
        f"height = {height}",
        f"width = {width}",
        f"tileset = {starting}:{tileset_resource_key}",
        "",
        "[set]",
    ]

    if set_resource_key is not None:
        lines.append(f"source = {set_resource_key}")
    else:
        lines.extend(_format_data_lines(set_tile_ids))

    lines.extend([
        "",
        "[setaboveplayer]",
    ])

    if above_player_resource_key is not None:
        lines.append(f"source = {above_player_resource_key}")
    elif above_player_tile_ids is not None:
        lines.extend(_format_data_lines(above_player_tile_ids))
    else:
        lines.append("source =")

    for obj in objects:
        obj_source = f"{obj.type.lower()}"

        lines.extend([
            "",
            f"[object:{obj.name}]",
            f"source = {obj_source}",
            f"position = {obj.x}, {obj.y}",
        ])
        if obj.sprite_id is not None and "sprite" not in obj.instance_attr.keys():
            lines.append(f"sprite = sprite_{obj.sprite_id}")
        for property_name, property_value in obj.instance_attr.items():
            lines.append(f"{property_name} = {property_value}")
        
    lines.append("")
    Path(ini_path).write_text("\n".join(lines))


def write_tileset_ini(
        ini_path: Path,
        bitmap_resource_key: str,
        tile_width: int,
        tile_height: int,
        tiles: dict[int, TsxTile],
        animations: dict[int, list[AnimationFrame]],
) -> None:
    lines = [
        "[tileset]",
        f"source = {bitmap_resource_key}",
        f"tilewidth = {tile_width}",
        f"tileheight = {tile_height}",
    ]

    tile_ids = sorted(set(tiles) | set(animations))
    for tile_id in tile_ids:
        tile = tiles.get(tile_id)
        block = [
            "",
            f"[id:{tile_id}]",
        ]
        if tile is not None:
            block.append(f"collision = {'true' if tile.collision else 'false'}")
            if tile.class_name:
                block.append(f"class = {tile.class_name}")

        for frame in animations.get(tile_id, []):
            block.append(f"animation = {frame.duration_ms}, {frame.tile_id}")

        lines.extend(block)

    lines.append("")
    Path(ini_path).write_text("\n".join(lines))


def write_dat(dat_path: Path, tile_ids: list[list[int]]) -> None:
    lines = [",".join(str(tile_id) for tile_id in row) + "," for row in tile_ids]
    Path(dat_path).write_text("\n".join(lines) + "\n")
