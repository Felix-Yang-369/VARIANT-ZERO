#include "VZInputSettings.h"
#include "GameFramework/PlayerInput.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
const TArray<FVZInputSpec>& FVZInputSettings::Specs()
{
    static const TArray<FVZInputSpec> S={
        {TEXT("VZForward"),TEXT("向前"),EKeys::W,FKey(),1},{TEXT("VZForward"),TEXT("向后"),EKeys::S,FKey(),-1},
        {TEXT("VZRight"),TEXT("向左"),EKeys::A,FKey(),-1},{TEXT("VZRight"),TEXT("向右"),EKeys::D,FKey(),1},
        {TEXT("VZJump"),TEXT("跳跃"),EKeys::SpaceBar,EKeys::Gamepad_FaceButton_Bottom},
        {TEXT("VZSprint"),TEXT("冲刺"),EKeys::LeftShift,EKeys::Gamepad_LeftThumbstick},
        {TEXT("VZDodge"),TEXT("闪避"),EKeys::LeftControl,EKeys::Gamepad_FaceButton_Right},
        {TEXT("VZInteract"),TEXT("交互"),EKeys::E,EKeys::Gamepad_FaceButton_Left},
        {TEXT("VZScan"),TEXT("扫描/标记"),EKeys::Q,EKeys::Gamepad_FaceButton_Top},
        {TEXT("VZPulse"),TEXT("脉冲"),EKeys::LeftMouseButton,EKeys::Gamepad_RightTrigger},
        {TEXT("VZShield"),TEXT("护盾"),EKeys::RightMouseButton,EKeys::Gamepad_LeftTrigger},
        {TEXT("VZCompanionSkill"),TEXT("伙伴技能"),EKeys::One,EKeys::Gamepad_RightShoulder},
        {TEXT("VZOrder"),TEXT("技能落点"),EKeys::G,EKeys::Gamepad_LeftShoulder},
        {TEXT("VZRecall"),TEXT("召回"),EKeys::R,EKeys::Gamepad_RightThumbstick},
        {TEXT("VZEncounter"),TEXT("激活练习"),EKeys::T,EKeys::Gamepad_Special_Right},
        {TEXT("VZCompanionSkill2"),TEXT("伙伴二技能"),EKeys::Two,EKeys::Gamepad_DPad_Left},
        {TEXT("VZCompanionSkill3"),TEXT("伙伴三技能"),EKeys::Three,EKeys::Gamepad_DPad_Right}};
    return S;
}
FVZInputSettings::FVZInputSettings(){for(const auto& S:Specs()){Keyboard.Add(S.Keyboard);Pad.Add(S.Pad);}}
bool FVZInputSettings::Assign(int32 I,FKey Key,bool Gamepad,FString& Error)
{
    if(!Specs().IsValidIndex(I)||!Key.IsValid()||Key.IsAxis1D()||Key.IsAxis2D()||Key.IsAxis3D()||Key.IsGamepadKey()!=Gamepad||(Gamepad&&Specs()[I].Axis!=0))
    {Error=TEXT("请选择对应设备上的按钮；摇杆轴保持默认。");return false;}
    if(Key==EKeys::Escape||Key==EKeys::Gamepad_Special_Left||Key==EKeys::F1||Key==EKeys::F5||Key==EKeys::F6||Key==EKeys::F9||Key==EKeys::F11||Key==EKeys::C||Key==EKeys::K||Key==EKeys::Tilde)
    {Error=TEXT("此键保留给菜单或开发工具，请换一个键。");return false;}
    auto& Keys=Gamepad?Pad:Keyboard;
    for(int32 J=0;J<Keys.Num();++J)if(J!=I&&Keys[J]==Key){Error=FString::Printf(TEXT("此键已用于“%s”，请先修改该动作。"),Specs()[J].Label);return false;}
    Keys[I]=Key;return true;
}
bool FVZInputSettings::Save(const FString& Path) const
{
    FConfigFile File;File.Read(Path);
    int32 Version=1;File.GetInt(TEXT("Bindings"),TEXT("Version"),Version);if(Version!=1 && Version!=2)return false;
    File.SetString(TEXT("Bindings"),TEXT("Version"),TEXT("2"));
    for(int32 I=0;I<Specs().Num();++I){File.SetString(TEXT("Bindings"),*FString::Printf(TEXT("Keyboard%d"),I),*Keyboard[I].ToString());File.SetString(TEXT("Bindings"),*FString::Printf(TEXT("Pad%d"),I),*Pad[I].ToString());}
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(Path),true);
    return File.Write(Path);
}
bool FVZInputSettings::Load(const FString& Path)
{
    FConfigFile File;File.Read(Path);int32 Version=1;File.GetInt(TEXT("Bindings"),TEXT("Version"),Version);if(Version!=1 && Version!=2)return false;FVZInputSettings Candidate;
    for(int32 I=0;I<Specs().Num();++I)
    {
        FString K,P;
        if(File.GetString(TEXT("Bindings"),*FString::Printf(TEXT("Keyboard%d"),I),K))Candidate.Keyboard[I]=FKey(*K);
        if(File.GetString(TEXT("Bindings"),*FString::Printf(TEXT("Pad%d"),I),P))Candidate.Pad[I]=FKey(*P);
    }
    if(Version==1)
    {
        // Keep all old bindings. Only new actions choose an unused fallback if their default was already taken.
        for(int32 I=15;I<Specs().Num();++I)
        {
            auto Resolve=[&](TArray<FKey>& Keys,const TArray<FKey>& Choices)
            {
                bool Conflict=false;for(int32 J=0;J<I;++J)Conflict|=Keys[J]==Keys[I];
                if(!Conflict)return true;
                for(const auto& Key:Choices)if(!Keys.Contains(Key)){Keys[I]=Key;return true;}return false;
            };
            if(!Resolve(Candidate.Keyboard,{EKeys::Two,EKeys::Three,EKeys::Four,EKeys::Five,EKeys::Six,EKeys::Seven,EKeys::Eight,EKeys::Nine,EKeys::Zero,EKeys::Y,EKeys::U,EKeys::I,EKeys::O,EKeys::P,EKeys::H,EKeys::J,EKeys::L}) || !Resolve(Candidate.Pad,{EKeys::Gamepad_DPad_Left,EKeys::Gamepad_DPad_Right,EKeys::Gamepad_DPad_Up,EKeys::Gamepad_DPad_Down}))return false;
        }
    }
    // Validate the final layout as a whole, allowing legitimate swaps across multiple saved keys.
    FString Error;
    for(int32 I=0;I<Specs().Num();++I)if(!Candidate.Assign(I,Candidate.Keyboard[I],false,Error)||(Specs()[I].Axis==0&&!Candidate.Assign(I,Candidate.Pad[I],true,Error)))return false;
    *this=Candidate;return true;
}
void FVZInputSettings::Apply(UPlayerInput* Input) const
{
    if(!Input)return;
    const auto Actions=Input->ActionMappings;const auto Axes=Input->AxisMappings;
    for(const auto& M:Actions)for(const auto& S:Specs())if(!S.Axis&&M.ActionName==S.Id){Input->RemoveActionMapping(M);break;}
    for(const auto& M:Axes)if(!M.Key.IsGamepadKey()&&(M.AxisName==TEXT("VZForward")||M.AxisName==TEXT("VZRight")))Input->RemoveAxisMapping(M);
    for(int32 I=0;I<Specs().Num();++I)
    {const auto& S=Specs()[I];if(S.Axis)Input->AddAxisMapping(FInputAxisKeyMapping(S.Id,Keyboard[I],S.Axis));else{Input->AddActionMapping(FInputActionKeyMapping(S.Id,Keyboard[I]));Input->AddActionMapping(FInputActionKeyMapping(S.Id,Pad[I]));}}
    Input->ForceRebuildingKeyMaps(false);
}
FString FVZInputSettings::KeyName(const TCHAR* Id) const
{for(int32 I=0;I<Specs().Num();++I)if(FString(Specs()[I].Id)==Id)return Keyboard[I].GetDisplayName().ToString();return Id;}
