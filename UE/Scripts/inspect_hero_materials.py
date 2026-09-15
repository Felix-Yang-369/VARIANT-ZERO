import unreal,json
from pathlib import Path
mesh=unreal.load_asset('/Game/ParagonLtBelica/Characters/Heroes/Belica/Meshes/Belica')
report=[]
for slot in mesh.get_editor_property('materials'):
    m=slot.material_interface
    names=unreal.MaterialEditingLibrary.get_vector_parameter_names(m)
    report.append({'slot':str(slot.material_slot_name),'material':m.get_path_name(),'vectors':[str(n) for n in names]})
Path(r'D:\Projects\VariantZeroUE\Docs\hero-material-inspection.json').write_text(json.dumps(report,indent=2),encoding='utf8')
