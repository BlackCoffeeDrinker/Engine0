"""Parse a Tiled `.tsx` external tileset definition into simple Python structures."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from xml.etree import ElementTree


@dataclass
class AnimationFrame:
    tile_id: int
    duration_ms: int


@dataclass
class TsxTile:
    tile_id: int
    class_name: str | None = None
    collision: bool = False


@dataclass
class TsxTileset:
    image_path: Path
    tile_width: int
    tile_height: int
    columns: int
    tile_count: int
    animations: dict[int, list[AnimationFrame]] = field(default_factory=dict)
    tiles: dict[int, TsxTile] = field(default_factory=dict)


def parse_tsx(tsx_path: Path) -> TsxTileset:
    tsx_path = Path(tsx_path)
    tree = ElementTree.parse(tsx_path)
    root = tree.getroot()

    if root.tag != "tileset":
        raise ValueError(f"{tsx_path}: not a Tiled tileset file (root tag is {root.tag!r})")

    tile_width = int(root.get("tilewidth"))
    tile_height = int(root.get("tileheight"))
    columns = int(root.get("columns"))
    tile_count = int(root.get("tilecount"))

    image_el = root.find("image")
    if image_el is None:
        raise ValueError(f"{tsx_path}: no <image> element found (only single-image tilesets are supported)")

    source = image_el.get("source")
    if not source:
        raise ValueError(f"{tsx_path}: <image> element has no source attribute")

    image_path = (tsx_path.parent / source).resolve()

    animations: dict[int, list[AnimationFrame]] = {}
    tiles: dict[int, TsxTile] = {}
    for tile_el in root.findall("tile"):
        tile_id = int(tile_el.get("id"))

        class_name = tile_el.get("type") or None

        collision = False
        properties_el = tile_el.find("properties")
        if properties_el is not None:
            for property_el in properties_el.findall("property"):
                # Only the `collision` property name is honored. The legacy `COLLIDE`
                # property found on some tiles in the source data is a mistake and is
                # intentionally ignored.
                if (property_el.get("name") or "").lower() == "collision":
                    collision = (property_el.get("value") or "").lower() in ("true", "1", "yes")

        if class_name is not None or collision:
            tiles[tile_id] = TsxTile(tile_id=tile_id, class_name=class_name, collision=collision)

        animation_el = tile_el.find("animation")
        if animation_el is None:
            continue

        frames = [
            AnimationFrame(tile_id=int(frame_el.get("tileid")), duration_ms=int(frame_el.get("duration")))
            for frame_el in animation_el.findall("frame")
        ]
        if frames:
            animations[tile_id] = frames

    return TsxTileset(
        image_path=image_path,
        tile_width=tile_width,
        tile_height=tile_height,
        columns=columns,
        tile_count=tile_count,
        animations=animations,
        tiles=tiles,
    )
