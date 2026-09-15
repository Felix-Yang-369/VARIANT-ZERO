"""Add a bounded exterior backdrop to a copy; retain the original art test map."""
import unreal,math,json
from pathlib import Path
ROOT=Path(r'D:\Projects\VariantZeroUE');OLD='/Game/VariantZero/Maps/P0_Conservatory';NEW=OLD+'_Foundation'
lib=unreal.EditorAssetLibrary;levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if lib.does_asset_exist(NEW):raise RuntimeError('Candidate already exists; inspect before revising')
if not lib.duplicate_asset(OLD,NEW):raise RuntimeError('Could not preserve source map')
if not levels.load_level(NEW):raise RuntimeError('Could not load copied map')
cube=unreal.load_asset('/Engine/BasicShapes/Cube');sphere=unreal.load_asset('/Engine/BasicShapes/Sphere')
ground=unreal.load_asset('/Game/VariantZero/Environment/Materials/M_Mineral');leaf=unreal.load_asset('/Game/VariantZero/Environment/Materials/M_LivingLeaf')
def mesh(name,pos,scale,asset,material,collision=False):
    a=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*pos));a.set_actor_label(name);a.static_mesh_component.set_static_mesh(asset);a.static_mesh_component.set_material(0,material);a.set_actor_scale3d(unreal.Vector(*scale));a.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS if collision else unreal.CollisionEnabled.NO_COLLISION)
mesh('Exterior ground',(0,0,-160),(500,500,2),cube,ground,False)
for i in range(32):
    a=i*math.tau/32;r=6500+1800*math.sin(i*2.4)
    mesh('Distant ecological ridge',(math.cos(a)*r,math.sin(a)*r,-200),(30+7*math.sin(i),35,12+5*math.cos(i*1.7)),sphere,leaf)
fog=actors.spawn_actor_from_class(unreal.ExponentialHeightFog,unreal.Vector(0,0,-180));fog.component.set_editor_property('fog_density',.018);fog.component.set_editor_property('fog_height_falloff',.25)
if not levels.save_current_level():raise RuntimeError('Save failed')
(ROOT/'Docs/foundation-environment.json').write_text(json.dumps({'map':NEW,'preserved':OLD,'status':'P0 backdrop candidate; primitive hills are not final realistic environment assets'},indent=2),encoding='utf-8')
