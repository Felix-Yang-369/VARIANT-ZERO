import unreal,json
from pathlib import Path
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);levels.load_level('/Game/VariantZero/Maps/P0_Conservatory_Ecology')
data={'constructor':str(unreal.Rotator(0,90,0)),'actors':[]}
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if a.get_actor_label()=='CC0 cultivation fern':
        origin,extent=a.get_actor_bounds(False)
        data['actors'].append({'rotation':str(a.get_actor_rotation()),'location':str(a.get_actor_location()),'origin':str(origin),'extent':str(extent)})
Path(r'D:\Projects\VariantZeroUE\Docs\ecology-rotations.json').write_text(json.dumps(data,indent=2),encoding='utf8')
