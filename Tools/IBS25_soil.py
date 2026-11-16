"""Three bounded official CC0 1K maps. No payment, plugin or credentials."""
import hashlib
import json
from pathlib import Path
import shutil
import urllib.request
from IBS25_pbr import dimensions

ROOT=Path(__file__).with_name('IBS25_sources')/'soil'
def download():
    assert shutil.disk_usage(ROOT.parent).free > 40*1024**3+48*1024**2
    ROOT.mkdir(exist_ok=True)
    rows=[]
    for channel,suffix,ext in [('basecolor','diff','jpg'),('normal_dx','nor_dx','png'),('roughness','rough','jpg')]:
        name=f'brown_mud_leaves_01_{suffix}_1k.{ext}'
        url=f'https://dl.polyhaven.org/file/ph-assets/Textures/{ext}/1k/brown_mud_leaves_01/{name}'
        path=ROOT/name
        if path.exists():
            data=path.read_bytes()
        else:
            with urllib.request.urlopen(url,timeout=45) as r:
                assert r.url.startswith('https://dl.polyhaven.org/')
                data=r.read(16*1024**2+1)
        assert len(data)<=16*1024**2 and dimensions(data)==(1024,1024)
        if not path.exists():
            with path.open('xb') as f: f.write(data)
        rows.append(dict(channel=channel,filename=name,url=url,bytes=len(data),sha256=hashlib.sha256(data).hexdigest(),
                         license='CC0-1.0',license_url='https://polyhaven.com/license',
                         source='https://polyhaven.com/a/brown_mud_leaves_01',author='Rob Tuytel',resolution=1024))
    receipt=ROOT/'receipt.json'
    content=json.dumps(rows,indent=2)+'\n'
    if receipt.exists() and receipt.read_text()!=content: raise RuntimeError('Receipt changed')
    receipt.write_text(content)
    print(json.dumps({'files':3,'bytes':sum(r['bytes'] for r in rows)}))

def imported():
    import unreal as ue
    import IBS25_author as a
    rows=json.loads((ROOT/'receipt.json').read_text())
    output={}
    for row in rows:
        file=ROOT/row['filename']
        assert hashlib.sha256(file.read_bytes()).hexdigest()==row['sha256']
        name='T_R2_Soil_'+row['channel']
        dest='/Game/IB_S25/Textures/'+name
        if ue.EditorAssetLibrary.does_asset_exist(dest):
            texture=ue.load_asset(dest)
            assert a.owned(texture)
            assert ue.EditorAssetLibrary.get_metadata_tag(texture,'IBS25SourceSHA')==row['sha256']
        else:
            task=ue.AssetImportTask()
            task.filename=str(file); task.destination_path='/Game/IB_S25/Textures';task.destination_name=name
            task.automated=True;task.save=False;task.replace_existing=False
            ue.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
            texture=ue.load_asset(dest)
            assert isinstance(texture,ue.Texture2D)
            texture.srgb=row['channel']=='basecolor'
            texture.max_texture_size=1024
            if row['channel']=='normal_dx':
                texture.compression_settings=ue.TextureCompressionSettings.TC_NORMALMAP
                texture.flip_green_channel=False
            ue.EditorAssetLibrary.set_metadata_tag(texture,'IBS25SourceSHA',row['sha256'])
            a.stamp(texture)
        output[row['channel']]=texture
    return output

if __name__=='__main__': download()
