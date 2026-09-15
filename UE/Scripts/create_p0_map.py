"""Run inside UE Python commandlet. Creates a separate graybox map; never alters template maps."""
import unreal
path = '/Game/VariantZero/Maps/P0_Greenhouse'
if unreal.EditorAssetLibrary.does_asset_exist(path):
    unreal.log('VZ P0 map already exists; preserving existing level')
else:
    subsystem = unreal.get_editor_subsystem(unreal.LevelEditorSubsystem)
    if not subsystem.new_level(path):
        raise RuntimeError('Could not create P0 map')
    world = unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
    mode = unreal.load_class(None, '/Script/VariantZeroUE.VZPrototypeMode')
    if not mode:
        raise RuntimeError('Native module is not loaded')
    world.get_world_settings().set_editor_property('default_game_mode', mode)
    unreal.get_editor_subsystem(unreal.EditorActorSubsystem).spawn_actor_from_class(unreal.PlayerStart, unreal.Vector(0,0,120))
    if not subsystem.save_current_level():
        raise RuntimeError('Could not save P0 map')
    unreal.log('VZ_P0_MAP_CREATED')
