import unreal,json
from pathlib import Path
root=Path(r'D:\Projects\VariantZeroUE')
t=unreal.AssetImportTask();t.filename=str(root/'ArtSource/Audio/Exploration/SW_BeyondRoots.wav');t.destination_path='/Game/VariantZero/Audio';t.destination_name='SW_BeyondRoots';t.automated=True;t.save=True;t.replace_existing=True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([t])
s=unreal.load_asset('/Game/VariantZero/Audio/SW_BeyondRoots')
assert isinstance(s,unreal.SoundWave)
s.set_editor_property('looping',True)
s.set_editor_property('virtualization_mode',unreal.VirtualizationMode.PLAY_WHEN_SILENT)
assert unreal.EditorAssetLibrary.save_loaded_asset(s)
for name in ['SW_Conservatory','SW_Alert']:
    a=unreal.load_asset('/Game/VariantZero/Audio/'+name)
    a.set_editor_property('virtualization_mode',unreal.VirtualizationMode.PLAY_WHEN_SILENT)
    unreal.EditorAssetLibrary.save_loaded_asset(a)
(root/'Docs/exploration-audio-import.json').write_text(json.dumps(dict(asset=s.get_path_name(),looping=True,virtualization='PLAY_WHEN_SILENT')),encoding='utf-8')
print('VZ_EXPLORATION_IMPORT: PASS')
