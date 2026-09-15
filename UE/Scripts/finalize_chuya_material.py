import unreal
p='/Game/VariantZero/Characters/Chuya/'
mesh=unreal.load_asset(p+'SK_Chuya');material=unreal.load_asset(p+'M_Chuya')
assert mesh and material
unreal.MaterialEditingLibrary.set_material_usage(material,unreal.MaterialUsage.MATUSAGE_SKELETAL_MESH)
unreal.MaterialEditingLibrary.recompile_material(material)
slots=mesh.get_editor_property('materials');slot=slots[0];slot.material_interface=material;slots[0]=slot;mesh.set_editor_property('materials',slots)
unreal.EditorAssetLibrary.save_loaded_asset(material,False);unreal.EditorAssetLibrary.save_loaded_asset(mesh,False)
assert mesh.get_editor_property('materials')[0].material_interface==material
unreal.log('VZ_CHUYA_MATERIAL_SAVED '+str(mesh.get_editor_property('materials')[0].material_interface))
