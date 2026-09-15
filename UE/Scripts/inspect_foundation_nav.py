import unreal,json
from pathlib import Path
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem);result={}
for path in ['/Game/VariantZero/Maps/P0_Conservatory','/Game/VariantZero/Maps/P0_Conservatory_Foundation']:
    assert levels.load_level(path)
    entries=[]
    for a in actors.get_all_level_actors():
        if 'Nav' in a.get_class().get_name() or a.get_actor_label() in ['Mineral foundation','Navigation test planter','Exterior ground']:
            origin,extent=a.get_actor_bounds(False)
            entries.append(dict(name=a.get_actor_label(),kind=a.get_class().get_name(),location=str(a.get_actor_location()),origin=str(origin),extent=str(extent)))
    result[path]=entries
Path(r'D:\Projects\VariantZeroUE\Docs\nav-comparison.json').write_text(json.dumps(result,indent=2),encoding='utf-8')
