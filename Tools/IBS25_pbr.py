"""Obtain only six official Poly Haven CC0 2K texture maps (no API/key/plugin).

Run with Python to download; run in UE via exec(open(...).read()) with
IBS25_IMPORT_PBR=1 to import into /Game/IB_S25/Textures. Tree is NOT substituted.
The source directory is fixed under this script; existing bytes are never replaced.
"""
import hashlib
import json
import os
from pathlib import Path
import shutil
import struct
import urllib.request

ROOT = Path(__file__).resolve().parent / "IBS25_sources"
LICENSE = "https://polyhaven.com/license"
LIMIT = 32 * 1024 * 1024
ASSETS = {"asphalt": "asphalt_02", "paving": "concrete_pavement"}
MAPS = {"basecolor": ("diff", "jpg"), "normal_dx": ("nor_dx", "png"), "roughness": ("rough", "jpg")}


def sources():
    for role, asset in ASSETS.items():
        for channel, (suffix, ext) in MAPS.items():
            filename = f"{asset}_{suffix}_2k.{ext}"
            yield {"key":f"{role}_{channel}","filename":filename,
                   "url":f"https://dl.polyhaven.org/file/ph-assets/Textures/{ext}/2k/{asset}/{filename}",
                   "source":f"https://polyhaven.com/a/{asset}","license":LICENSE,
                   "license_id":"CC0-1.0","resolution":2048,"format":ext}


def dimensions(data):
    if data.startswith(b"\x89PNG\r\n\x1a\n"):
        return struct.unpack(">II",data[16:24])
    if not data.startswith(b"\xff\xd8"):
        raise ValueError("Expected PNG/JPEG, not an error/HTML response")
    pos = 2
    while pos < len(data):
        if data[pos] != 255:
            raise ValueError("Malformed JPEG")
        while data[pos] == 255:
            pos += 1
        marker = data[pos]
        pos += 1
        if marker in (0xD8,0xD9):
            continue
        size = int.from_bytes(data[pos:pos+2],"big")
        if marker in (0xC0,0xC1,0xC2):
            height,width = struct.unpack(">HH",data[pos+3:pos+7])
            return width,height
        if size < 2:
            raise ValueError("Malformed JPEG segment")
        pos += size
    raise ValueError("No JPEG size marker")


def download():
    # Budget: at most 192 MiB plus source manifest, preserving the agreed 40 GiB reserve.
    if shutil.disk_usage(ROOT.parent).free < 40*1024**3 + 6*LIMIT:
        raise RuntimeError("Insufficient shared SSD reserve")
    ROOT.mkdir(exist_ok=True)
    inventory = []
    for source in sources():
        path = ROOT/source["filename"]
        if path.exists():
            data = path.read_bytes()
        else:
            request = urllib.request.Request(source["url"],headers={"User-Agent":"IMPACT-S25-local-asset-preparation/1.0"})
            with urllib.request.urlopen(request,timeout=45) as response:
                if not response.url.startswith("https://dl.polyhaven.org/"):
                    raise ValueError("Unexpected download destination")
                data = response.read(LIMIT+1)
        if len(data) > LIMIT or dimensions(data) != (2048,2048):
            raise ValueError("Invalid texture size or resolution: " + path.name)
        sha = hashlib.sha256(data).hexdigest()
        # Existing files must match the previous receipt, if present.
        manifest = ROOT/"manifest.json"
        if path.exists() and manifest.exists():
            prior = {r["filename"]:r for r in json.loads(manifest.read_text())}
            if path.name not in prior or prior[path.name]["sha256"] != sha:
                raise ValueError("Existing source changed: " + path.name)
        if not path.exists():
            with path.open("xb") as stream:
                stream.write(data)
        inventory.append({**source,"bytes":len(data),"sha256":sha})
    content = json.dumps(inventory,indent=2)+"\n"
    manifest = ROOT/"manifest.json"
    if manifest.exists() and manifest.read_text() != content:
        raise FileExistsError("Source receipt changed; inspect manually")
    if not manifest.exists():
        manifest.write_text(content,encoding="utf-8")
    print(json.dumps({"files":len(inventory),"bytes":sum(r["bytes"] for r in inventory),"status":"DOWNLOADED_NOT_IMPORTED"}))


def import_into_unreal():
    import unreal as ue
    inventory = json.loads((ROOT/"manifest.json").read_text())
    required = {r["key"]:r for r in sources()}
    if set(r["key"] for r in inventory) != set(required) or len(inventory) != 6:
        raise ValueError("Unexpected source inventory")
    for row in inventory:
        if any(row[k] != required[row["key"]][k] for k in required[row["key"]]):
            raise ValueError("Source metadata mismatch")
        data = (ROOT/row["filename"]).read_bytes()
        if hashlib.sha256(data).hexdigest() != row["sha256"] or dimensions(data) != (2048,2048):
            raise ValueError("Source integrity mismatch")
    output = {}
    for row in inventory:
        name = "T_"+row["key"]
        path = "/Game/IB_S25/Textures/"+name
        texture = ue.load_asset(path) if ue.EditorAssetLibrary.does_asset_exist(path) else None
        if texture:
            if ue.EditorAssetLibrary.get_metadata_tag(texture,"IBS25SourceSHA") != row["sha256"]:
                raise RuntimeError("Foreign/modified texture occupies " + path)
        else:
            task = ue.AssetImportTask()
            task.filename = str(ROOT/row["filename"])
            task.destination_path = "/Game/IB_S25/Textures"
            task.destination_name = name
            task.automated,task.save,task.replace_existing = True,False,False
            ue.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
            texture = ue.load_asset(path)
            if not isinstance(texture,ue.Texture2D):
                raise RuntimeError("Texture import failed: " + path)
            texture.set_editor_property("srgb",row["key"].endswith("basecolor"))
            if row["key"].endswith("normal_dx"):
                texture.set_editor_property("compression_settings",ue.TextureCompressionSettings.TC_NORMALMAP)
                texture.set_editor_property("flip_green_channel",False)
            texture.set_editor_property("max_texture_size",2048)
            ue.EditorAssetLibrary.set_metadata_tag(texture,"IBS25SourceSHA",row["sha256"])
            if not ue.EditorAssetLibrary.save_loaded_asset(texture):
                raise RuntimeError("Texture save failed")
        output[row["key"]] = path
    config = ROOT/"assets.json"
    content = json.dumps(output,indent=2)+"\n"
    if config.exists() and config.read_text() != content:
        raise FileExistsError("Preserving edited assets.json; add paths manually")
    if not config.exists():
        config.write_text(content,encoding="utf-8")
    ue.log("IBS25 PBR imported. Tree dependency still needs a licensed local mesh.")


if __name__ == "__main__":
    if os.environ.get("IBS25_IMPORT_PBR") == "1":
        import_into_unreal()
    else:
        download()
