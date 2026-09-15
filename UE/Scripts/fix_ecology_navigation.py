import unreal,json
from pathlib import Path
levels=unreal.get_editor_subsystem(unreal.LevelEditorSubsystem);actors=unreal.get_editor_subsystem(unreal.EditorActorSubsystem)
report={}
for path in ['/Game/VariantZero/Maps/P0_Conservatory_Foundation2','/Game/VariantZero/Maps/P0_Conservatory_Ecology']:
    assert levels.load_level(path)
    nav=next(a for a in actors.get_all_level_actors() if isinstance(a,unreal.RecastNavMesh))
    report[path]={}
    for prop in ['runtime_generation','force_rebuild_on_load','can_be_main_nav_data','agent_radius','agent_height']:
        report[path][prop]=str(nav.get_editor_property(prop))
    if path.endswith('_Ecology'):
        nav.set_editor_property('runtime_generation',unreal.RuntimeGenerationType.DYNAMIC)
        nav.set_editor_property('force_rebuild_on_load',True)
        world=unreal.get_editor_subsystem(unreal.UnrealEditorSubsystem).get_editor_world()
        report[path]['detour_after_rebuild']=unreal.VZEditorTools.rebuild_scene_navigation(world)
        if not report[path]['detour_after_rebuild']:raise RuntimeError('Navigation detour did not build')
        assert levels.save_current_level()
Path(r'D:\Projects\VariantZeroUE\Docs\ecology-navigation-settings.json').write_text(json.dumps(report,indent=2),encoding='utf8')
