import unreal,json
from pathlib import Path
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);levels.load_level('/Game/VariantZero/Maps/P0_Conservatory_Ecology')
report=[]
for a in unreal.get_editor_subsystem(unreal.EditorActorSubsystem).get_all_level_actors():
    if not isinstance(a,unreal.StaticMeshActor):continue
    c=a.static_mesh_component;relevant=c.get_editor_property('can_ever_affect_navigation')
    if relevant or a.get_actor_label()=='Institute terrain - visual boundary':
        origin,extent=a.get_actor_bounds(False)
        report.append({'name':a.get_actor_label(),'nav':relevant,'collision':str(c.get_collision_enabled()),'mesh':c.static_mesh.get_path_name(),'origin':str(origin),'extent':str(extent)})
Path(r'D:\Projects\VariantZeroUE\Docs\ecology-collision.json').write_text(json.dumps(report,indent=2),encoding='utf8')
