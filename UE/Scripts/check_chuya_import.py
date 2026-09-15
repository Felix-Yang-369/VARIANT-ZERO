import unreal
dest='/Game/VariantZero/Characters/Chuya/'
for n in ['SK_Chuya_Skeleton','SK_Chuya','A_Chuya_Idle','A_Chuya_Walk']:
    obj=unreal.load_asset(dest+n)
    unreal.log('VZ_CHECK '+n+' '+str(obj))
    assert obj,'Missing '+n
    if n!='SK_Chuya_Skeleton':
        skeleton=obj.get_editor_property('skeleton')
        unreal.log('VZ_SKELETON '+str(skeleton))
        assert skeleton,'Missing persistent skeleton on '+n
unreal.log('VZ_CHUYA_RELOAD_PASS')
