import unreal
sub=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
assert sub.load_level('/Game/VariantZero/Maps/P0_Conservatory_Foundation')
actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
found=0
for actor in actors.get_all_level_actors():
    if actor.get_actor_label()=='Exterior ground':
        actor.static_mesh_component.set_collision_enabled(unreal.CollisionEnabled.NO_COLLISION);found+=1
assert found==1
assert sub.save_current_level()
