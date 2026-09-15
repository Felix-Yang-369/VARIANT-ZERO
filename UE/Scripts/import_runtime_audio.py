import unreal,json
from pathlib import Path
ROOT=Path(r'D:\Projects\VariantZeroUE');SRC=ROOT/'ArtSource/Audio/P0';DEST='/Game/VariantZero/Audio'
tools=unreal.AssetToolsHelpers.get_asset_tools();result=[]
for p in SRC.glob('*.wav'):
    task=unreal.AssetImportTask();task.filename=str(p);task.destination_path=DEST;task.destination_name=p.stem;task.automated=True;task.save=True;task.replace_existing=True
    tools.import_asset_tasks([task]);assets=[unreal.load_asset(s) for s in task.imported_object_paths]
    sound=next(a for a in assets if isinstance(a,unreal.SoundWave))
    sound.set_editor_property('looping',p.stem in ['SW_Conservatory','SW_Alert'])
    unreal.EditorAssetLibrary.save_loaded_asset(sound);result.append(sound.get_path_name())
(ROOT/'Docs/audio-import.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
