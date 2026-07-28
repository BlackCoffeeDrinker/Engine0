"""Parse a Tiled `.tmx` orthogonal tile map into simple Python structures."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from xml.etree import ElementTree


@dataclass
class TmxLayer:
    name: str
    width: int
    height: int
    gids: list[int] = field(default_factory=list)

    def gid_at(self, x: int, y: int) -> int:
        return self.gids[y * self.width + x]


@dataclass
class TmxMap:
    width: int
    height: int
    tile_width: int
    tile_height: int
    firstgid: int
    tsx_path: Path
    layers: list[TmxLayer] = field(default_factory=list)


def _parse_csv_data(data_text: str) -> list[int]:
    return [int(v) for v in data_text.replace("\n", "").split(",") if v.strip() != ""]


def parse_tmx(tmx_path: Path) -> TmxMap:
    tmx_path = Path(tmx_path)
    tree = ElementTree.parse(tmx_path)
    root = tree.getroot()

    if root.tag != "map":
        raise ValueError(f"{tmx_path}: not a Tiled map file (root tag is {root.tag!r})")

    if root.get("orientation") != "orthogonal":
        raise ValueError(f"{tmx_path}: only orthogonal maps are supported")

    width = int(root.get("width"))
    height = int(root.get("height"))
    tile_width = int(root.get("tilewidth"))
    tile_height = int(root.get("tileheight"))

    tileset_el = root.find("tileset")
    if tileset_el is None:
        raise ValueError(f"{tmx_path}: no <tileset> reference found")

    source = tileset_el.get("source")
    if not source:
        raise ValueError(f"{tmx_path}: only external .tsx tileset references are supported")

    firstgid = int(tileset_el.get("firstgid"))
    tsx_path = (tmx_path.parent / source).resolve()

    layers: list[TmxLayer] = []
    for layer_el in root.findall("layer"):
        layer_width = int(layer_el.get("width"))
        layer_height = int(layer_el.get("height"))
        data_el = layer_el.find("data")
        if data_el is None:
            raise ValueError(f"{tmx_path}: layer {layer_el.get('name')!r} has no <data>")
        if data_el.get("encoding") != "csv":
            raise ValueError(f"{tmx_path}: only csv-encoded layer data is supported")

        gids = _parse_csv_data(data_el.text or "")
        if len(gids) != layer_width * layer_height:
            raise ValueError(
                f"{tmx_path}: layer {layer_el.get('name')!r} has {len(gids)} cells, "
                f"expected {layer_width * layer_height}"
            )

        layers.append(TmxLayer(name=layer_el.get("name", ""), width=layer_width, height=layer_height, gids=gids))

    if not layers:
        raise ValueError(f"{tmx_path}: no tile layers found")

    return TmxMap(
        width=width,
        height=height,
        tile_width=tile_width,
        tile_height=tile_height,
        firstgid=firstgid,
        tsx_path=tsx_path,
        layers=layers,
    )
