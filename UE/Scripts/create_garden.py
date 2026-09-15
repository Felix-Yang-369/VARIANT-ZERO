import unreal,json,math,random
from pathlib import Path
ROOT=Path(r'D:\Projects\VariantZeroUE');DEST='/Game/VariantZero/Maps/P0_Conservatory_Garden'
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
if unreal.EditorAssetLibrary.does_asset_exist(DEST):raise RuntimeError('Garden map exists; preserve it and inspect before another revision')
assert unreal.EditorAssetLibrary.duplicate_asset('/Game/VariantZero/Maps/P0_Conservatory_Ecology',DEST)
assert levels.load_level(DEST)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
for a in list(actors.get_all_level_actors()):
    label=a.get_actor_label();p=a.get_actor_location()
    if label=='Low garden enclosure' and p.y>1000 and abs(p.x)<300:actors.destroy_actor(a)
    if label=='Institute terrain - visual boundary':actors.destroy_actor(a)
    if label.startswith('CC0') and p.y>1200 and abs(p.x)<1700:actors.destroy_actor(a)
    if isinstance(a,unreal.NavMeshBoundsVolume):a.set_actor_location(unreal.Vector(0,2700,300),False,False);a.set_actor_scale3d(unreal.Vector(1,3.7,1))
def prop(label,pos,scale,material='M_Ivory',shape='Cube',collision=True,yaw=0):
    a=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*pos),unreal.Rotator(pitch=0,yaw=yaw,roll=0));a.set_actor_label(label)
    c=a.static_mesh_component;c.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/'+shape));c.set_material(0,unreal.load_asset('/Game/VariantZero/Environment/Materials/'+material));a.set_actor_scale3d(unreal.Vector(*scale));c.set_collision_enabled(unreal.CollisionEnabled.QUERY_AND_PHYSICS if collision else unreal.CollisionEnabled.NO_COLLISION)
    return a
floor=prop('Garden walkable ground',(0,4050,-30),(26,57,.6),'M_Soil');floor.static_mesh_component.set_material(0,unreal.load_asset('/Game/VariantZero/Ecology/M_forest_ground_04'))
for x in [-1350,1350]:prop('Garden perimeter',(x,4050,130),(.3,58,2.6),'M_LivingLeaf')
prop('Garden far boundary',(0,6950,130),(27,.3,2.6),'M_LivingLeaf')
for y in range(1500,6900,600):
    prop('Path stone',(0,y,1),(4,5,.02),'M_Mineral',collision=False)
for x,sx in [(-750,11),(550,3)]:prop('Garden partition',(x,3700,110),(sx,.2,2.2),'M_Ivory')
gate=prop('Restoration shortcut',(100,3700,110),(6,.2,2.2),'M_DormantLeaf');gate.tags=['VZGardenShortcut']
for x,y in [(-750,4250),(750,4250),(0,6500)]:
    prop('Heliostat plinth',(x,y,20),(2.2,2.2,.4),'M_Brass','Cylinder')
    for side in [-1,1]:prop('Canopy column',(x+side*220,y,220),(.15,.15,4.4),'M_Ivory','Cylinder')
    prop('Canopy frame',(x,y,440),(4.6,.18,.18),'M_Brass')
prop('Garden archive',(0,2500,50),(2.5,1,.95),'M_Ivory')
prop('Pollen receiver',(0,5100,65),(2.6,2.6,1.3),'M_LivingLeaf','Cylinder')
for i in range(8):
    t=i*math.tau/8;prop('Receiver petals',(90*math.cos(t),5100+90*math.sin(t),145),(1,.5,.15),'M_Petal','Sphere',False,i*45)
random.seed(41)
for i in range(80):
    x=random.choice([-1,1])*random.uniform(1050,1280);y=random.uniform(1700,6800)
    a=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(x,y,1),unreal.Rotator(pitch=0,yaw=random.uniform(0,360),roll=0));a.set_actor_label('Garden CC0 fern');a.static_mesh_component.set_static_mesh(unreal.load_asset('/Game/VariantZero/Ecology/SM_fern_02'));a.set_actor_scale3d(unreal.Vector(.7,.7,1.4));a.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION);a.static_mesh_component.set_editor_property('can_ever_affect_navigation',False)
assert unreal.VZEditorTools.rebuild_scene_navigation(world)
assert levels.save_current_level()
(ROOT/'Docs/garden-scene.json').write_text(json.dumps({'map':DEST,'walkable_extension_m':[26,57],'mirrors':3,'ferns':80,'status':'authored gameplay blockout; runtime navigation verification required'},indent=2))
unreal.log('VZ_GARDEN_SCENE_PASS')
