from pathlib import Path
root = Path(__file__).resolve().parents[1]
engine = root / 'Config/DefaultEngine.ini'
text = engine.read_text('utf-8-sig')
text = text.replace('GameName=TP_ThirdPersonBP', 'GameName=VariantZeroUE')
text = text.replace('/Game/ThirdPerson/Lvl_ThirdPerson.Lvl_ThirdPerson', '/Game/VariantZero/Maps/P0_Greenhouse.P0_Greenhouse')
text = text.replace('/Game/ThirdPerson/Blueprints/BP_ThirdPersonGameMode.BP_ThirdPersonGameMode_C', '/Script/VariantZeroUE.VZPrototypeMode')
engine.write_text(text, encoding='utf-8')
config = root / 'Config/DefaultInput.ini'
text = config.read_text('utf-8-sig')
text = text.replace('DefaultPlayerInputClass=/Script/EnhancedInput.EnhancedPlayerInput','DefaultPlayerInputClass=/Script/Engine.PlayerInput')
text = text.replace('DefaultInputComponentClass=/Script/EnhancedInput.EnhancedInputComponent','DefaultInputComponentClass=/Script/Engine.InputComponent')
if 'ActionName="VZJump"' not in text:
    actions = {'VZJump':['SpaceBar','Gamepad_FaceButton_Bottom'],'VZSprint':['LeftShift','Gamepad_LeftThumbstick'],'VZDodge':['LeftControl','Gamepad_FaceButton_Right'],'VZInteract':['E','Gamepad_FaceButton_Left'],'VZScan':['Q','Gamepad_FaceButton_Top'],'VZSave':['F5'],'VZLoad':['F9'],'VZSlot':['F6'],'VZCheckpoint':['C'],'VZRespawn':['K']}
    for name, keys in actions.items():
        for key in keys:
            text += f'+ActionMappings=(ActionName="{name}",Key={key},bShift=False,bCtrl=False,bAlt=False,bCmd=False)\n'
    for name,key,scale in [('VZForward','W',1),('VZForward','S',-1),('VZForward','Gamepad_LeftY',1),('VZRight','D',1),('VZRight','A',-1),('VZRight','Gamepad_LeftX',1),('VZLookX','MouseX',1),('VZLookY','MouseY',-1),('VZLookX','Gamepad_RightX',1),('VZLookY','Gamepad_RightY',-1)]:
        text += f'+AxisMappings=(AxisName="{name}",Key={key},Scale={scale}.000000)\n'
config.write_text(text, encoding='utf-8')
print('P0 map and prototype bindings configured.')
