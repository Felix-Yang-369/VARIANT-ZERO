import unreal
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
assert levels.load_level('/Game/VariantZero/Maps/P0_Conservatory_Garden')
if any(a.get_actor_label()=='Garden distant ground' for a in actors.get_all_level_actors()):
    raise RuntimeError('Finishing pass already applied; use repair_garden_navigation.py for navigation only')
for a in list(actors.get_all_level_actors()):
    p=a.get_actor_location()
    if a.get_actor_label().startswith('CC0') and (abs(p.x)>1700 or p.y>1400 or p.y < -1700):
        actors.destroy_actor(a)

def cube(label,loc,scale,material):
    a=actors.spawn_actor_from_class(unreal.StaticMeshActor,unreal.Vector(*loc),unreal.Rotator())
    a.set_actor_label(label);a.set_actor_scale3d(unreal.Vector(*scale))
    c=a.static_mesh_component;c.set_static_mesh(unreal.load_asset('/Engine/BasicShapes/Cube'))
    c.set_material(0,unreal.load_asset(material));c.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION)
    c.set_editor_property('can_ever_affect_navigation',False)
    return a

cube('Garden distant ground',(0,3000,-90),(400,400,1),'/Game/VariantZero/Ecology/M_forest_ground_04')
for x in [-850,850]:
    cube('Dry irrigation channel',(x,5250,2),(1.5,29,.035),'/Game/VariantZero/Environment/Materials/M_Mineral')
    water=cube('Restored irrigation water',(x,5250,5),(1.2,29,.025),'/Game/VariantZero/Environment/Materials/M_Biolight')
    water.tags=['VZGardenWater'];water.set_actor_hidden_in_game(True)
world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
assert unreal.VZEditorTools.rebuild_scene_navigation(world)
assert levels.save_current_level()
unreal.log('VZ_GARDEN_FINISH_PASS')
