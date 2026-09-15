"""Rebuild candidate from the verified authoring recipe; do not clone navigation data."""
from pathlib import Path
ROOT=Path(r'D:\Projects\VariantZeroUE')
source=(ROOT/'Scripts/create_conservatory.py').read_text(encoding='utf-8')
source=source.replace("PATH='/Game/VariantZero/Maps/P0_Conservatory'","PATH='/Game/VariantZero/Maps/P0_Conservatory_Foundation2'").replace('conservatory-build.json','foundation2-build.json')
exec(compile(source,'create_conservatory_foundation2','exec'))
NEW='/Game/VariantZero/Maps/P0_Conservatory_Foundation2';OLD='/Game/VariantZero/Maps/P0_Conservatory';levels=sub;lib=unreal.EditorAssetLibrary
backdrop=(ROOT/'Scripts/create_foundation_environment.py').read_text(encoding='utf-8')
exec(compile(backdrop[backdrop.index("cube=unreal.load_asset"):],'foundation_backdrop','exec'))
for actor in actors.get_all_level_actors():
    if actor.get_actor_label() in ['Exterior ground','Distant ecological ridge']:
        actor.static_mesh_component.set_editor_property('can_ever_affect_navigation',False)
assert levels.save_current_level()
