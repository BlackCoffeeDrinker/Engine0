from __future__ import annotations

from pathlib import Path

from tsx_reader import AnimationFrame

# Default `[resourcemap]` keys already declared in `example/res/game.ini`.
DEFAULT_SET_RESOURCE_KEY = "wh1-0-dat"
DEFAULT_TILESET_RESOURCE_KEY = "samsettileset"


def write_ini(
    ini_path: Path,
    width: int,
    height: int,
    tile_width: int,
    tile_height: int,
    animations: dict[int, list[AnimationFrame]],
    set_resource_key: str = DEFAULT_SET_RESOURCE_KEY,
    tileset_resource_key: str = DEFAULT_TILESET_RESOURCE_KEY,
) -> None:
    lines = [
        "[map]",
        f"height = {height}",
        f"width = {width}",
        f"set = {set_resource_key}",
        "",
        "[tileset]",
        f"source = {tileset_resource_key}",
        f"tilewidth = {tile_width}",
        f"tileheight = {tile_height}",
    ]

    for tile_id in sorted(animations):
        frames = animations[tile_id]
        frame_ids = ", ".join(str(frame.tile_id) for frame in frames)
        durations = ", ".join(str(frame.duration_ms) for frame in frames)
        lines.extend([
            "",
            f"[tile:{tile_id}]",
            "type = animated",
            f"frame = {frame_ids}",
            f"duration = {durations}",
        ])

    lines.append("")
    Path(ini_path).write_text("\n".join(lines))


def write_dat(dat_path: Path, tile_ids: list[list[int]]) -> None:
    lines = [",".join(str(tile_id) for tile_id in row) + "," for row in tile_ids]
    Path(dat_path).write_text("\n".join(lines) + "\n")
