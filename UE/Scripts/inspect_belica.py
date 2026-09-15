import unreal, json
from pathlib import Path
root = '/Game/ParagonLtBelica/Characters/Heroes/Belica'
mesh = unreal.load_asset(root + '/Meshes/Belica')
actor = unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(unreal.SkeletalMeshActor, unreal.Vector())
component = actor.get_component_by_class(unreal.SkeletalMeshComponent)
component.set_skeletal_mesh_asset(mesh)
report = {'mesh': mesh.get_path_name(), 'skeleton': mesh.get_editor_property('skeleton').get_path_name(),
          'materials': [str(m.material_slot_name) for m in mesh.get_editor_property('materials')],
          'bones': [str(component.get_bone_name(i)) for i in range(component.get_num_bones())], 'animations': {}}
for name in ['Idle_Relaxed', 'Jog_Fwd', 'Jump_Start', 'Jump_Apex', 'Jump_PreLand']:
    a = unreal.load_asset(root + '/Animations/' + name)
    report['animations'][name] = {'length': a.get_play_length(), 'skeleton': a.get_editor_property('skeleton').get_path_name()}
Path(r'D:\Projects\VariantZeroUE\Docs\belica-inspection.json').write_text(json.dumps(report, indent=2), encoding='utf-8')
unreal.log('VZ_BELICA_INSPECT PASS')
