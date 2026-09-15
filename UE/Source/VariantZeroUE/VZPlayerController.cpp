#include "VZPlayerController.h"
#include "VZPrototype.h"
#include "VZPrologue.h"
#include "VZState.h"
#include "VZCombat.h"
#include "VZAudioDirector.h"
#include "VZGarden.h"
#include "VZTravel.h"
#include "VZMap.h"
#include "VZRoster.h"
#include "VZChuyaStory.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "NavigationSystem.h"
#include "EngineUtils.h"
#include "Components/AudioComponent.h"
#include "GameFramework/PlayerInput.h"
#include "AudioDevice.h"
#include "Engine/GameViewportClient.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "Misc/CommandLine.h"
#include "HAL/FileManager.h"
#include "HighResScreenshot.h"
#include "Framework/Application/SlateApplication.h"
#include "Widgets/SCompoundWidget.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Text/STextBlock.h"
#include "UObject/UObjectIterator.h"
#include "Blueprint/WidgetTree.h"
#include "Components/TextBlock.h"

class SVZPauseMenu : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SVZPauseMenu) {} SLATE_END_ARGS()
    TWeakObjectPtr<AVZPlayerController> Owner;
    int32 Selected=0;
    double LastAnalog=0;
    TSharedPtr<SScrollBox> RecordScroll;
    void Construct(const FArguments&, AVZPlayerController* PC)
    {
        Owner=PC;
        TSharedRef<SVerticalBox> Rows=SNew(SVerticalBox);
        Rows->AddSlot().AutoHeight().Padding(10,8)[SNew(STextBlock).Text(FText::FromString(TEXT("未完成的春天 · 已暂停"))).Font(FCoreStyle::GetDefaultFontStyle("Bold",26))];
        Rows->AddSlot().AutoHeight().Padding(10,4)[SNew(STextBlock).Text(FText::FromString(TEXT("温室 / 研究所终端"))).ColorAndOpacity(FLinearColor(.55f,.85f,.72f))];
        Rows->AddSlot().AutoHeight().Padding(10,4)[SNew(SEditableTextBox)
            .Visibility_Lambda([this](){return Owner.IsValid()&&Owner->RosterPage?EVisibility::Visible:EVisibility::Collapsed;})
            .HintText(FText::FromString(TEXT("伙伴名字（最多 16 字），点击下方保存名字")))
            .Text_Lambda([this](){return FText::FromString(Owner.IsValid()?Owner->RenameDraft:FString());})
            .OnTextChanged_Lambda([this](const FText& T){if(Owner.IsValid())Owner->RenameDraft=T.ToString();})];
        for(int32 I=0;I<10;++I)
        {
            Rows->AddSlot().AutoHeight().Padding(8,3)
            [SNew(SButton).IsFocusable(false).ContentPadding(FMargin(16,7))
                .ButtonColorAndOpacity_Lambda([this,I](){return Selected==I?FSlateColor(FLinearColor(.15f,.5f,.34f)):FSlateColor(FLinearColor(.07f,.12f,.11f));})
                .OnHovered_Lambda([this,I](){Selected=I;})
                .OnClicked_Lambda([this,I](){if(Owner.IsValid())Owner->ActivateMenuRow(I);return FReply::Handled();})
                [SNew(STextBlock).Font(FCoreStyle::GetDefaultFontStyle("Regular",18)).Text_Lambda([this,I](){return FText::FromString(Owner.IsValid()?Owner->MenuLabel(I):FString());})]];
        }
        Rows->AddSlot().AutoHeight().Padding(10,8)[SNew(SBox).MaxDesiredHeight(220)
            [SAssignNew(RecordScroll,SScrollBox)+SScrollBox::Slot()
                [SNew(STextBlock).AutoWrapText(true).Font_Lambda([this](){return FCoreStyle::GetDefaultFontStyle("Regular",Owner.IsValid()?Owner->DialogueFontSize:18);}).Text_Lambda([this](){return FText::FromString(Owner.IsValid()?Owner->MenuStatus():FString());}).ColorAndOpacity(FLinearColor(.8f,.9f,.84f))]]];
        Rows->AddSlot().AutoHeight().Padding(10,5)[SNew(STextBlock).Text(FText::FromString(TEXT("↑↓ / 方向键选择 · Enter / A 确认 · Esc / B 返回\n滚轮、PageUp/Down 或右摇杆滚动记录"))).Font(FCoreStyle::GetDefaultFontStyle("Regular",13))];
        ChildSlot[SNew(SBorder).BorderBackgroundColor(FLinearColor(.008f,.022f,.018f,.95f)).HAlign(HAlign_Center).VAlign(VAlign_Center)
            [SNew(SBox).WidthOverride(600)[Rows]]];
    }
    virtual bool SupportsKeyboardFocus() const override { return true; }
    virtual FReply OnKeyDown(const FGeometry&,const FKeyEvent& Event) override
    {
        if(!Owner.IsValid())return FReply::Unhandled();
        const FKey K=Event.GetKey();
        if(Owner->CaptureDevice>=0)
        {if(!Event.IsRepeat())Owner->CaptureBinding(K);return FReply::Handled();}
        if((K==EKeys::PageUp || K==EKeys::PageDown) && RecordScroll.IsValid())
        {RecordScroll->SetScrollOffset(FMath::Max(0.f,RecordScroll->GetScrollOffset()+(K==EKeys::PageUp?-140.f:140.f)));return FReply::Handled();}
        if(K==EKeys::Escape||K==EKeys::Gamepad_FaceButton_Right||K==EKeys::Gamepad_Special_Left)
        {Owner->MenuBack();return FReply::Handled();}
        if(K==EKeys::Up||K==EKeys::Gamepad_DPad_Up){Selected=(Selected+9)%10;return FReply::Handled();}
        if(K==EKeys::Down||K==EKeys::Gamepad_DPad_Down){Selected=(Selected+1)%10;return FReply::Handled();}
        if((K==EKeys::Enter||K==EKeys::SpaceBar||K==EKeys::Gamepad_FaceButton_Bottom)&&!Event.IsRepeat())
        {Owner->ActivateMenuRow(Selected);return FReply::Handled();}
        return FReply::Handled();
    }
    virtual FReply OnAnalogValueChanged(const FGeometry&,const FAnalogInputEvent& E) override
    {
        const double Now=FPlatformTime::Seconds();
        if(E.GetKey()==EKeys::Gamepad_RightY && FMath::Abs(E.GetAnalogValue())>.25f && RecordScroll.IsValid())
        {RecordScroll->SetScrollOffset(FMath::Max(0.f,RecordScroll->GetScrollOffset()-E.GetAnalogValue()*16));return FReply::Handled();}
        if(E.GetKey()==EKeys::Gamepad_LeftY&&FMath::Abs(E.GetAnalogValue())>.5f&&Now-LastAnalog>.22f)
        {Selected=(Selected+(E.GetAnalogValue()>0?9:1))%10;LastAnalog=Now;}
        return FReply::Handled();
    }
    virtual FReply OnPreviewMouseButtonDown(const FGeometry&,const FPointerEvent& Event) override
    {
        if(Owner.IsValid()&&Owner->CaptureDevice>=0){Owner->CaptureBinding(Event.GetEffectingButton());return FReply::Handled();}
        return FReply::Unhandled();
    }
};

AVZPlayerController::AVZPlayerController(){PrimaryActorTick.bTickEvenWhenPaused=true;}
FString AVZPlayerController::SettingsPath() const
{
    return Verification?FPaths::ProjectSavedDir()/TEXT("Automation/MenuSettings.ini"):FPaths::GeneratedConfigDir()/TEXT("VariantZeroSettings.ini");
}
void AVZPlayerController::ApplyVolume()
{
    if(auto Device=GetWorld()->GetAudioDevice())Device->SetTransientPrimaryVolume(MasterVolume);
    for(TActorIterator<AVZAudioDirector> It(GetWorld());It;++It)It->ApplyMix();
}
void AVZPlayerController::LoadPresentationSettings()
{
    MasterVolume=MusicVolume=EffectsVolume=1;DialogueFontSize=18;
    FConfigFile Settings;Settings.Read(SettingsPath());
    Settings.GetFloat(TEXT("Audio"),TEXT("MasterVolume"),MasterVolume);
    Settings.GetFloat(TEXT("Audio"),TEXT("MusicVolume"),MusicVolume);
    Settings.GetFloat(TEXT("Audio"),TEXT("EffectsVolume"),EffectsVolume);
    Settings.GetInt(TEXT("Accessibility"),TEXT("DialogueFontSize"),DialogueFontSize);
    for(float* Value:{&MasterVolume,&MusicVolume,&EffectsVolume})*Value=FMath::IsFinite(*Value)?FMath::Clamp(*Value,0.f,1.f):1.f;
    if(DialogueFontSize!=18 && DialogueFontSize!=24 && DialogueFontSize!=30)DialogueFontSize=18;
}
bool AVZPlayerController::SavePresentationSettings()
{
    FConfigFile Settings;Settings.Read(SettingsPath());
    Settings.SetFloat(TEXT("Audio"),TEXT("MasterVolume"),MasterVolume);
    Settings.SetFloat(TEXT("Audio"),TEXT("MusicVolume"),MusicVolume);
    Settings.SetFloat(TEXT("Audio"),TEXT("EffectsVolume"),EffectsVolume);
    Settings.SetString(TEXT("Accessibility"),TEXT("DialogueFontSize"),*FString::FromInt(DialogueFontSize));
    IFileManager::Get().MakeDirectory(*FPaths::GetPath(SettingsPath()),true);
    return Settings.Write(SettingsPath());
}
void AVZPlayerController::BeginPlay()
{
    Super::BeginPlay();
    Verification=FParse::Param(FCommandLine::Get(),TEXT("VZMenuTest"));
    if(!Verification)LoadPresentationSettings();
    MasterVolume=FMath::IsFinite(MasterVolume)?FMath::Clamp(MasterVolume,0.f,1.f):1.f;
    ApplyVolume();VerificationStart=FPlatformTime::Seconds();
    if(!Verification)Bindings.Load(SettingsPath());
    Bindings.Apply(PlayerInput);
}
void AVZPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();
    InputComponent->BindAction("VZPause",IE_Pressed,this,&AVZPlayerController::ToggleMenu).bExecuteWhenPaused=true;
}
void AVZPlayerController::ToggleMenu()
{
    if(Menu.IsValid()){CloseMenu();return;}
    auto* P=Cast<AVZEcologist>(GetPawn());if(!P||!GetWorld()->GetGameViewport())return;
    P->Prologue->Dismiss();
    P->AimingOrder=false;P->OrderAimValid=false;P->StopPulse();P->StopSprint();P->StopJumping();P->GetCharacterMovement()->StopMovementImmediately();
    FlushPressedKeys();PendingAction=-1;SelectedSlot=P->ManualSlot;BindingsPage=false;ArchivePage=false;RosterPage=false;PresentationPage=false;CaptureDevice=-1;
    if(!SetPause(true))return;
    Menu=SNew(SVZPauseMenu,this);
    GetWorld()->GetGameViewport()->AddViewportWidgetContent(Menu.ToSharedRef(),100);
    bShowMouseCursor=true;
    FInputModeUIOnly Mode;Mode.SetWidgetToFocus(Menu);Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);SetInputMode(Mode);
    FSlateApplication::Get().SetKeyboardFocus(Menu,EFocusCause::SetDirectly);
}
void AVZPlayerController::CloseMenu()
{
    if(MapWidget){MapWidget->RemoveFromParent();MapWidget=nullptr;}
    if(!Menu.IsValid())return;
    GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(Menu.ToSharedRef());Menu.Reset();PendingAction=-1;
    SetPause(false);bShowMouseCursor=false;SetInputMode(FInputModeGameOnly());FlushPressedKeys();CaptureDevice=-1;BindingsPage=false;ArchivePage=false;RosterPage=false;PresentationPage=false;
    if(auto* P=Cast<AVZEcologist>(GetPawn())){P->StopPulse();P->StopSprint();}
}
void AVZPlayerController::OpenMap()
{
    auto* P=Cast<AVZEcologist>(GetPawn());if(!P||!P->Prologue->Enabled()){ArchiveText=TEXT("当前场景未开放探索地图。");return;}
    if(!Menu.IsValid()||MapWidget)return;
    MapWidget=CreateWidget<UVZMapWidget>(this);
    if(!MapWidget)return;
    ArchiveText=TEXT("选择已知地点进行标记。移动会自动保存，战斗中无法快速移动。");
    Menu->SetVisibility(EVisibility::Hidden);MapWidget->AddToViewport(101);
    FInputModeUIOnly Mode;Mode.SetWidgetToFocus(MapWidget->TakeWidget());Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);SetInputMode(Mode);MapWidget->SetKeyboardFocus();
}
void AVZPlayerController::CloseMap()
{
    if(!MapWidget)return;
    MapWidget->RemoveFromParent();MapWidget=nullptr;
    if(Menu.IsValid())
    {
        Menu->SetVisibility(EVisibility::Visible);
        FInputModeUIOnly Mode;Mode.SetWidgetToFocus(Menu);SetInputMode(Mode);
        FSlateApplication::Get().SetKeyboardFocus(Menu,EFocusCause::SetDirectly);
    }
}
void AVZPlayerController::EndPlay(const EEndPlayReason::Type Reason)
{
    if(MapWidget){MapWidget->RemoveFromParent();MapWidget=nullptr;}
    if(Menu.IsValid()&&GetWorld()->GetGameViewport())GetWorld()->GetGameViewport()->RemoveViewportWidgetContent(Menu.ToSharedRef());
    Menu.Reset();Super::EndPlay(Reason);
}
FString AVZPlayerController::MenuLabel(int32 Row) const
{
    const auto* P=Cast<AVZEcologist>(GetPawn());if(!P)return FString();
    if(RosterPage)
    {
        if(Row==7&&P->State()->State.Companions.IsValidIndex(RosterIndex))
        {
            const int32 L=P->State()->State.Companions[RosterIndex].Level;
            if(L>=20)return TEXT("培养已满级：20 级");
            return FString::Printf(TEXT("%s → %d 级（%d 点 / 可用 %d）"),PendingAction==100+L?TEXT("确认培养"):TEXT("培养"),L+1,L,FVZRosterRules::GrowthPoints(P->State()->State));
        }
        if(Row==6&&P->State()->State.Companions.IsValidIndex(RosterIndex))
            return P->State()->State.Party.Contains(P->State()->State.Companions[RosterIndex].IndividualId)?TEXT("研究所整备：留守并休整队伍"):TEXT("研究所整备：入队并休整队伍");
        static const TCHAR* Labels[]={TEXT("返回研究记录"),TEXT("上一只伙伴"),TEXT("下一只伙伴"),TEXT("保存伙伴名字"),TEXT("出战顺序：向前一位"),TEXT("出战顺序：向后一位"),TEXT("查看该物种的共同经历"),TEXT("查看初芽伙伴故事"),TEXT("名字与顺序操作说明"),TEXT("继续探索")};
        return Row>=0&&Row<10?Labels[Row]:TEXT("");
    }
    if(PresentationPage)
    {
        switch(Row)
        {
        case 0:return TEXT("返回暂停菜单");
        case 1:return FString::Printf(TEXT("总音量：%d%%  → 减少 10%%"),FMath::RoundToInt(MasterVolume*100));
        case 2:return FString::Printf(TEXT("音乐音量：%d%%  → 减少 10%%"),FMath::RoundToInt(MusicVolume*100));
        case 3:return FString::Printf(TEXT("音效音量：%d%%  → 减少 10%%"),FMath::RoundToInt(EffectsVolume*100));
        case 4:return FString::Printf(TEXT("对话与记录字号：%d  → 切换"),DialogueFontSize);
        case 5:return PendingAction==5?TEXT("再次确认：恢复音画默认设置"):TEXT("恢复音画默认设置");
        case 6:return TEXT("试听：育生杖脉冲");
        case 7:return TEXT("试听：生态修复");
        case 8:return TEXT("查看设置说明");
        case 9:return TEXT("继续探索");
        default:return FString();
        }
    }
    if(ArchivePage)
    {
        static const TCHAR* Labels[]={TEXT("返回暂停菜单"),TEXT("快速移动：原初研究所"),TEXT("快速移动：无访花庭"),TEXT("研究记录：温室"),TEXT("研究记录：花庭"),TEXT("共同经历：初芽与同类伙伴"),TEXT("共同经历：菌翼蛾"),TEXT("研究材料与修复奖励"),TEXT("打开地图 / 标记目标"),TEXT("继续探索")};
        if(Row<0 || Row>=10)return FString();
        if(Row==2 && FVZGardenRules::Stage(P->State()->State)==0)return TEXT("无访花庭 · 尚未解锁");
        return Row==5?TEXT("伙伴管理 / 名字 / 出战顺序"):Labels[Row];
    }
    if(BindingsPage)
    {
        const auto& S=FVZInputSettings::Specs()[BindingIndex];
        switch(Row)
        {
        case 0:return TEXT("← 上一个动作");
        case 1:return FString::Printf(TEXT("%d / %d  %s   → 下一个动作"),BindingIndex+1,FVZInputSettings::Specs().Num(),S.Label);
        case 2:return CaptureDevice==0?TEXT("等待键鼠按键… Esc 取消"):TEXT("键鼠：")+Bindings.Keyboard[BindingIndex].GetDisplayName().ToString()+TEXT("   → 修改");
        case 3:return S.Axis?TEXT("手柄移动：左摇杆（固定轴）"):CaptureDevice==1?TEXT("等待手柄按钮… 左菜单键取消"):TEXT("手柄：")+Bindings.Pad[BindingIndex].GetDisplayName().ToString()+TEXT("   → 修改");
        case 4:return TEXT("恢复此动作默认按键");
        case 5:return PendingAction==5?TEXT("再次确认：恢复全部默认按键"):TEXT("恢复全部默认按键");
        case 6:return TEXT("查看操作说明");
        case 7:return TEXT("返回暂停菜单");
        case 8:return TEXT("继续探索");
        case 9:return TEXT("返回暂停菜单");
        }
    }
    if(PendingAction==Row)return TEXT("再次确认：")+(Row==8?FString(TEXT("退出游戏（不会额外保存）")):Row==2?FString(TEXT("覆盖选中手动槽")):FString(TEXT("读取并返回存档检查点")));
    switch(Row)
    {
    case 0:return TEXT("继续探索");
    case 1:return FString::Printf(TEXT("手动存档槽：%d / 3   → 切换"),SelectedSlot);
    case 2:return TEXT("保存到选中槽位");
    case 3:return TEXT("读取选中槽位");
    case 4:return TEXT("读取自动存档");
    case 5:return P->State()->State.StoryDifficulty?TEXT("难度：故事   → 标准"):TEXT("难度：标准   → 故事");
    case 6:return TEXT("声音 / 阅读设置");
    case 7:return TEXT("按键设置 / 操作说明");
    case 8:return TEXT("退出游戏");
    case 9:return TEXT("地图 / 研究记录 / 快速移动");
    default:return FString();
    }
}
FString AVZPlayerController::MenuStatus() const
{
    const auto* P=Cast<AVZEcologist>(GetPawn());if(!P)return FString();
    if(RosterPage)
    {
        const auto& S=P->State()->State;
        if(!S.Companions.IsValidIndex(RosterIndex))return TEXT("暂无伙伴");
        const auto& C=S.Companions[RosterIndex];const int32 Slot=S.Party.IndexOfByKey(C.IndividualId);
        return FString::Printf(TEXT("%d / %d · %s · %s · 等级 %d\n%s\n%s\n%s"),RosterIndex+1,S.Companions.Num(),*C.Name,*C.SpeciesId,C.Level,Slot==INDEX_NONE?TEXT("留在研究所"):*FString::Printf(TEXT("出战技能槽 %d"),Slot+1),*P->State()->LastMessage,*ArchiveText);
    }
    if(ArchivePage)return ArchiveText;
    if(PresentationPage)return P->State()->LastMessage+TEXT("\n音乐与音效可单独静音；音量到零后再次点击恢复 100%。字号提供 18 / 24 / 30，应用于新打开的对话和研究记录。");
    const FString Path=P->State()->SlotPath(SelectedSlot);
    const FDateTime Stamp=IFileManager::Get().GetTimeStamp(*Path);
    const FString Info=Stamp==FDateTime::MinValue()?TEXT("空槽位"):TEXT("存档时间（UTC）：")+Stamp.ToString(TEXT("%Y-%m-%d %H:%M"));
    return Info+TEXT("\n")+P->State()->LastMessage+TEXT("\n存档保留任务与伙伴，读档返回检查点并重置本场战斗。");
}
void AVZPlayerController::ActivateMenuRow(int32 Row)
{
    auto* P=Cast<AVZEcologist>(GetPawn());if(!P||!Menu.IsValid())return;
    auto* S=P->State();
    if(RosterPage)
    {
        if(Row!=7)PendingAction=-1;
        if(Row==0){RosterPage=false;ArchiveText=TEXT("选择记录或地点。");return;}
        if(Row==1||Row==2){SelectCompanion(Row==1?-1:1);return;}
        if(Row==3){EditCompanion(true);return;}
        if(Row==4||Row==5){EditCompanion(false,Row==4?-1:1);return;}
        if(Row==6){ToggleCompanionDeployment();return;}
        if(Row==7&&S->State.Companions.IsValidIndex(RosterIndex))
        {
            const int32 L=S->State.Companions[RosterIndex].Level;
            if(L>=20){PendingAction=-1;S->LastMessage=TEXT("该伙伴已达到 20 级上限。");return;}
            if(PendingAction!=100+L)
            {
                PendingAction=100+L;
                ArchiveText=FString::Printf(TEXT("培养预览：生命 %.0f → %.0f，普攻 %.1f → %.1f。\n初芽技能 %.0f → %.0f；菌翼蛾治疗 %.0f → %.0f。\n消耗 %d 培养点；再次点击培养确认，其他操作取消。\n培养点来自温室修复、花庭供水和花庭报告，共 8 点；不消耗主线材料。"),FVZRosterRules::MaxHealth(L),FVZRosterRules::MaxHealth(L+1),FVZRosterRules::BasicDamage(L),FVZRosterRules::BasicDamage(L+1),FVZRosterRules::SkillDamage(L),FVZRosterRules::SkillDamage(L+1),FVZRosterRules::Healing(L),FVZRosterRules::Healing(L+1),L);
                return;
            }
            PendingAction=-1;TrainCompanion(L);return;
        }
        if(Row==8){ArchiveText=TEXT("输入名字后保存，最多 16 字。排序保留生命和冷却。\n在研究所培养台附近可入队或留守：至少携带 1 只、最多 3 只；整备恢复当前队伍生命与冷却，保留全部个体经历。战斗、倒地或执行指令时不可操作。\n花庭机关需要菌翼蛾，初芽故事需要原来的初芽；可快速移动回研究所重新入队。留守伙伴不在场景中运行 AI。\n手柄可调整队伍；自定义名字需要键盘输入。");return;}
        if(Row==9){CloseMenu();return;}
        return;
    }
    if(PresentationPage)
    {
        if(Row==0){PresentationPage=false;PendingAction=-1;return;}
        if(Row==9){CloseMenu();return;}
        if(Row==6 || Row==7){AVZAudioDirector::PlayCue(GetWorld(),Row==6?TEXT("Pulse"):TEXT("Repair"));return;}
        if(Row==8){S->LastMessage=TEXT("关闭音乐仍可听见工具音效。设置独立于角色存档；恢复默认不会改变按键或游戏进度。");return;}
        if(Row<1 || Row>5)return;
        if(Row==5 && PendingAction!=5){PendingAction=5;return;}
        PendingAction=-1;
        if(Row<=3){float& V=Row==1?MasterVolume:Row==2?MusicVolume:EffectsVolume;V=V<.05f?1.f:FMath::Max(0.f,V-.1f);}
        if(Row==4)DialogueFontSize=DialogueFontSize==18?24:DialogueFontSize==24?30:18;
        if(Row==5){MasterVolume=MusicVolume=EffectsVolume=1;DialogueFontSize=18;}
        ApplyVolume();S->LastMessage=SavePresentationSettings()?TEXT("音画设置已生效并保存。"):TEXT("设置已生效，但保存失败；下次启动可能无法恢复。");return;
    }
    if(ArchivePage)
    {
        if(Row==0){ArchivePage=false;return;}
        if(Row==5){RosterPage=true;SelectCompanion(0);return;}
        if(Row==1 || Row==2){TravelTo(Row==1?TEXT("institute"):TEXT("garden"));return;}
        if(Row>=3 && Row<=7){ArchiveText=FVZTravelRules::Record(S->State,Row-3);return;}
        if(Row==8){OpenMap();return;}
        if(Row==9){CloseMenu();return;}
        return;
    }
    if(BindingsPage)
    {
        if(CaptureDevice>=0)return;
        if(Row==0){BindingIndex=(BindingIndex+FVZInputSettings::Specs().Num()-1)%FVZInputSettings::Specs().Num();PendingAction=-1;return;}
        if(Row==1){BindingIndex=(BindingIndex+1)%FVZInputSettings::Specs().Num();PendingAction=-1;return;}
        if(Row==2||Row==3){if(Row==3&&FVZInputSettings::Specs()[BindingIndex].Axis)return;CaptureDevice=Row-2;PendingAction=-1;return;}
        if(Row==4||Row==5)
        {
            if(Row==5&&PendingAction!=5){PendingAction=5;return;}
            PendingAction=-1;FVZInputSettings Candidate=Row==5?FVZInputSettings():Bindings;FString Error;
            const auto& Spec=FVZInputSettings::Specs()[BindingIndex];
            if(Row==4&&(!Candidate.Assign(BindingIndex,Spec.Keyboard,false,Error)||(!Spec.Axis&&!Candidate.Assign(BindingIndex,Spec.Pad,true,Error)))){S->LastMessage=Error;return;}
            if(Candidate.Save(SettingsPath())){Bindings=Candidate;Bindings.Apply(PlayerInput);S->LastMessage=TEXT("默认按键已恢复并保存。");}else S->LastMessage=TEXT("按键保存失败，原设置保留。");return;
        }
        if(Row==6){S->LastMessage=TEXT("选择动作后修改按键。重复按键会提示冲突；Esc 与手柄左菜单键始终可返回。\n移动与镜头摇杆轴固定，动作按钮可以重设。右键护盾、伙伴技能和指令需松开再按。");return;}
        if(Row==7 || Row==9){BindingsPage=false;PendingAction=-1;return;}
        if(Row==8){CloseMenu();return;}
    }
    const bool Confirm=Row==3||Row==4||Row==8||(Row==2&&IFileManager::Get().FileExists(*S->SlotPath(SelectedSlot)));
    if(Confirm&&PendingAction!=Row){PendingAction=Row;return;}
    PendingAction=-1;
    switch(Row)
    {
    case 0:CloseMenu();break;
    case 1:SelectedSlot=SelectedSlot%3+1;P->ManualSlot=SelectedSlot;break;
    case 2:S->SaveSlot(SelectedSlot);break;
    case 3:case 4:if(S->LoadSlot(Row==4?0:SelectedSlot)){HasMapMarker=false;P->Respawn();P->RefreshEcology();}break;
    case 5:P->ToggleDifficulty();break;
    case 6:PresentationPage=true;S->LastMessage=TEXT("调整声音与阅读字号。");break;
    case 7:BindingsPage=true;CaptureDevice=-1;S->LastMessage=TEXT("选中动作，点击键鼠或手柄行开始重新绑定。");break;
    case 8:UKismetSystemLibrary::QuitGame(this,this,EQuitPreference::Quit,false);break;
    case 9:ArchivePage=true;ArchiveText=TEXT("选择地点快速移动，或查看已经获得的研究记录。未探索的区域不会提前显示故事结论。");break;
    }
}
void AVZPlayerController::MenuBack()
{
    if(MapWidget){CloseMap();return;}
    if(RosterPage){RosterPage=false;ArchiveText=TEXT("选择记录或地点。");return;}
    if(CaptureDevice>=0){CaptureDevice=-1;return;}
    if(PendingAction>=0){PendingAction=-1;return;}
    if(BindingsPage){BindingsPage=false;return;}
    if(ArchivePage){ArchivePage=false;return;}
    if(PresentationPage){PresentationPage=false;return;}
    CloseMenu();
}
bool AVZPlayerController::TravelTo(const FString& Destination)
{
    auto* P=Cast<AVZEcologist>(GetPawn());if(!P)return false;
    auto Fail=[this,P](const TCHAR* Message){ArchiveText=Message;P->State()->LastMessage=Message;return false;};
    if(!P->Garden->Enabled())return Fail(TEXT("此场景未开放快速移动。"));
    if(P->Vital->IsDown())return Fail(TEXT("倒地时无法快速移动。"));
    for(TActorIterator<AVZDisturbance> It(GetWorld());It;++It)if(It->Active)return Fail(TEXT("战斗中无法快速移动。请先平息异常或安全离开。"));
    if(P->Garden->Guardian && P->Garden->Guardian->Cleared && FVZGardenRules::Stage(P->State()->State)==1)
        return Fail(TEXT("花庭修复尚在结算。请先返回探索，等待供水恢复提示后再移动。"));
    auto Candidate=P->State()->State;
    if(!FVZTravelRules::Apply(Candidate,Destination))return Fail(TEXT("该移动点尚未解锁。"));
    auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());FNavLocation NavPosition;
    if(!Nav || !Nav->ProjectPointToNavigation(Candidate.Checkpoint,NavPosition,FVector(100,100,200)))return Fail(TEXT("目的地暂时无法安全到达，请稍后重试。"));
    if(!P->State()->Commit(Candidate))return Fail(TEXT("自动保存失败，已留在原处，进度未改变。"));
    HasMapMarker=false;CloseMenu();P->Respawn();SetControlRotation(FRotator(-10,90,0));
    P->State()->LastMessage=TEXT("已抵达移动点。队伍恢复，任务与材料保留。");return true;
}
void AVZPlayerController::SelectCompanion(int32 Delta)
{
    auto* P=Cast<AVZEcologist>(GetPawn());if(!P)return;
    const auto& All=P->State()->State.Companions;if(All.IsEmpty())return;
    RosterIndex=(FMath::Clamp(RosterIndex,0,All.Num()-1)+Delta+All.Num())%All.Num();
    const auto& C=All[RosterIndex];RenameDraft=C.Name;ArchiveText=TEXT("这个伙伴的经历：\n");
    const TPair<const TCHAR*,const TCHAR*> Memories[]={
        {TEXT("origin.first_companion"),TEXT("在温室与你第一次相遇。")},
        {TEXT("prologue.first_cultivation"),TEXT("在你的第一次培育中诞生。")},
        {TEXT("prologue.first_flower"),TEXT("陪你修复温室花床。")},
        {TEXT("prologue.pollinator_research"),TEXT("通过传粉研究加入队伍。")},
        {TEXT("prologue.first_pollination"),TEXT("帮助温室完成第一次传粉。")},
        {TEXT("garden.first_rain"),TEXT("共同见证花庭重新供水。")},
        {TEXT("story.chuya.first_rain"),TEXT("与你观察水土，寻找适合自己的生长之地。")}};
    bool Any=false;for(const auto& M:Memories)if(C.Memories.Contains(M.Key)){ArchiveText+=FString(M.Value)+TEXT("\n");Any=true;}
    if(!Any)ArchiveText+=TEXT("等待一起创造新的经历。\n");
    PendingAction=-1;
    if(C.IndividualId==FVZChuyaStoryRules::Protagonist(P->State()->State))ArchiveText+=TEXT("\n")+FVZChuyaStoryRules::Journal(P->State()->State);
}
bool AVZPlayerController::EditCompanion(bool Rename,int32 Direction)
{
    auto* P=Cast<AVZEcologist>(GetPawn());if(!P)return false;
    auto* S=P->State();if(!S->State.Companions.IsValidIndex(RosterIndex))return false;
    if(P->Vital->IsDown()){S->LastMessage=TEXT("倒地时不能修改伙伴。");return false;}
    for(TActorIterator<AVZDisturbance> It(GetWorld());It;++It)if(It->Active){S->LastMessage=TEXT("请先结束战斗再修改伙伴。");return false;}
    for(AVZCompanionProxy* C:P->Companions)if(IsValid(C)&&C->HasGroundOrder){S->LastMessage=TEXT("请先召回执行指令的伙伴。");return false;}
    const FGuid Id=S->State.Companions[RosterIndex].IndividualId;
    auto Candidate=S->State;
    const int32 OldSlot=Candidate.Party.IndexOfByKey(Id),NewSlot=OldSlot+Direction;
    if(!Rename&&(!P->Companions.IsValidIndex(OldSlot)||!P->Companions.IsValidIndex(NewSlot)||!IsValid(P->Companions[OldSlot])||!IsValid(P->Companions[NewSlot])||!Candidate.Party.IsValidIndex(NewSlot)||P->Companions[OldSlot]->IndividualId!=Id||P->Companions[NewSlot]->IndividualId!=Candidate.Party[NewSlot])){S->LastMessage=TEXT("无法移到该槽位；当前队伍保留。");return false;}
    if(!(Rename?FVZRosterRules::Rename(Candidate,Id,RenameDraft):FVZRosterRules::Move(Candidate,Id,Direction))){S->LastMessage=TEXT("未修改：名字需为 1–16 字且不含换行，顺序需在现有队伍范围内。");return false;}
    if(!S->Commit(Candidate))return false;
    if(!Rename)
    {
        P->Companions.Swap(OldSlot,NewSlot);
        float* CDs[]={&P->CompanionSkillCooldown,&P->CompanionSkillCooldown2,&P->CompanionSkillCooldown3};
        Swap(*CDs[OldSlot],*CDs[NewSlot]);
        for(int32 I=0;I<P->Companions.Num();++I)P->Companions[I]->FormationOffset=FVector(-180,100+I*90,0);
    }
    RenameDraft=S->State.Companions[RosterIndex].Name;S->LastMessage=TEXT("伙伴资料已自动保存；个体经历、生命和技能冷却保留。");return true;
}
bool AVZPlayerController::ToggleCompanionDeployment()
{
    auto* P=Cast<AVZEcologist>(GetPawn());if(!P)return false;
    auto* S=P->State();if(!S->State.Companions.IsValidIndex(RosterIndex))return false;
    if(P->Vital->IsDown()){S->LastMessage=TEXT("倒地时不能整备，请先返回检查点。");return false;}
    // Only this established cultivation station is a rest point, never a remote pause-menu heal.
    if(FVector::Dist2D(P->GetActorLocation(),FVector(-650,350,0))>400||FMath::Abs(P->GetActorLocation().Z-120)>220)
    {S->LastMessage=TEXT("请先回到研究所培养台附近整备（地图：培养台）。");return false;}
    for(TActorIterator<AVZDisturbance> It(GetWorld());It;++It)if(It->Active)
    {S->LastMessage=TEXT("战斗结束后才能整备。");return false;}
    if(P->AimingOrder||P->Prologue->DialogueOpen())
    {S->LastMessage=TEXT("请先结束对话或技能瞄准。");return false;}
    for(AVZCompanionProxy* C:P->Companions)if(IsValid(C)&&C->HasGroundOrder)
    {S->LastMessage=TEXT("请先召回执行指令的伙伴。");return false;}
    auto Candidate=S->State;const FGuid Id=Candidate.Companions[RosterIndex].IndividualId;
    const bool Deploy=!Candidate.Party.Contains(Id);
    if(!FVZRosterRules::SetDeployed(Candidate,Id,Deploy))
    {S->LastMessage=Deploy?TEXT("队伍已满，请先选择一只伙伴留守；最多携带 3 只。"):TEXT("请至少携带一只伙伴出发。");return false;}
    if(!S->Commit(Candidate))return false;
    P->Prologue->CancelPollination();P->RebuildParty();P->Vital->Restore();
    P->StopPulse();P->PulseCooldown=0;P->ShieldCooldown=0;P->DodgeCooldown=0;
    P->CompanionSkillCooldown=0;P->CompanionSkillCooldown2=0;P->CompanionSkillCooldown3=0;
    SelectCompanion(0);
    S->LastMessage=Deploy?TEXT("伙伴已入队；研究所整备完成，队伍恢复并自动保存。"):TEXT("伙伴已留守研究所；其余队伍恢复并自动保存。个体资料与故事经历保留。");
    return true;
}
bool AVZPlayerController::TrainCompanion(int32 ExpectedLevel)
{
    auto* P=Cast<AVZEcologist>(GetPawn());if(!P)return false;
    auto* S=P->State();if(!S->State.Companions.IsValidIndex(RosterIndex))return false;
    if(P->Vital->IsDown()||P->AimingOrder||P->Prologue->DialogueOpen())return false;
    if(FVector::Dist2D(P->GetActorLocation(),FVector(-650,350,0))>400||FMath::Abs(P->GetActorLocation().Z-120)>220)
    {S->LastMessage=TEXT("请回到研究所培养台附近培养伙伴。");return false;}
    for(TActorIterator<AVZDisturbance> It(GetWorld());It;++It)if(It->Active)
    {S->LastMessage=TEXT("战斗结束后才能培养。");return false;}
    for(auto C:P->Companions)if(IsValid(C)&&C->HasGroundOrder)
    {S->LastMessage=TEXT("请先召回执行指令的伙伴。");return false;}
    auto Candidate=S->State;const auto Id=Candidate.Companions[RosterIndex].IndividualId;
    if(!FVZRosterRules::Train(Candidate,Id,ExpectedLevel))
    {S->LastMessage=TEXT("培养未完成：培养点不足、已满级或等级已变化。请重新查看培养预览。");return false;}
    if(!S->Commit(Candidate))return false;
    P->RebuildParty();P->CompanionSkillCooldown=0;P->CompanionSkillCooldown2=0;P->CompanionSkillCooldown3=0;
    SelectCompanion(0);S->LastMessage=TEXT("培养完成并自动保存，伙伴队伍已休整。主线材料与共同经历保留。");return true;
}
bool AVZPlayerController::CaptureBinding(FKey Key)
{
    if(CaptureDevice<0)return false;
    if(Key==EKeys::Escape||Key==EKeys::Gamepad_Special_Left){CaptureDevice=-1;return false;}
    auto* P=Cast<AVZEcologist>(GetPawn());if(!P)return false;
    auto Candidate=Bindings;FString Error;
    if(!Candidate.Assign(BindingIndex,Key,CaptureDevice==1,Error)){P->State()->LastMessage=Error;return false;}
    if(!Candidate.Save(SettingsPath())){P->State()->LastMessage=TEXT("按键保存失败，原设置保留。");return false;}
    Bindings=Candidate;Bindings.Apply(PlayerInput);FlushPressedKeys();CaptureDevice=-1;P->State()->LastMessage=TEXT("按键已生效并保存。");return true;
}
void AVZPlayerController::Tick(float Dt)
{
    Super::Tick(Dt);
#if !UE_BUILD_SHIPPING
    if(!Verification)return;
    auto* P=Cast<AVZEcologist>(GetPawn());if(!P||!P->Disturbance)return;
    const double Elapsed=FPlatformTime::Seconds()-VerificationStart;
    auto Check=[this](const TCHAR* Label,bool Ok){VerificationOk &= Ok; if(!Ok)UE_LOG(LogTemp,Error,TEXT("VZ_MENU_CHECK: %s"),Label);};
    if(VerificationStep==0&&Elapsed>1)
    {
        P->StartEncounter();P->StartPulse();ToggleMenu();
        Check(TEXT("Menu.IsValid()&&UGameplayStatics::IsGamePaused(this)&&!P->PulseHeld"),Menu.IsValid()&&UGameplayStatics::IsGamePaused(this)&&!P->PulseHeld);
        PausedAttackClock=P->Disturbance->AttackClock;PausedPosition=P->GetActorLocation();
        VerificationStep=10;
    }
    else if(VerificationStep==10)
    {
        // Actors already scheduled in the opening frame can finish that frame before pause takes effect.
        PausedAttackClock=P->Disturbance->AttackClock;PausedPosition=P->GetActorLocation();VerificationStep=1;
    }
    else if(VerificationStep==1&&Elapsed>3)
    {
        Check(TEXT("P->Disturbance->AttackClock==PausedAttackClock&&P->GetActorLocation().Equals(PausedPosition)"),P->Disturbance->AttackClock==PausedAttackClock&&P->GetActorLocation().Equals(PausedPosition));
        // Exercise the same key handler used by Slate, including gamepad navigation and activation.
        if(!Menu.IsValid()){UE_LOG(LogTemp,Error,TEXT("VZ_MENU: FAIL menu unexpectedly closed"));FPlatformMisc::RequestExitWithStatus(true,1);return;}
        Menu->Selected=0; // Ignore the desktop cursor's initial hover position in this deterministic test.
        auto Send=[this](FKey K){auto KeepAlive=Menu;if(KeepAlive.IsValid())KeepAlive->OnKeyDown(FGeometry(),FKeyEvent(K,FModifierKeysState(),0,false,0,0));else VerificationOk=false;};
        Send(EKeys::Gamepad_DPad_Down);Send(EKeys::Gamepad_FaceButton_Bottom);
        Check(TEXT("SelectedSlot==2"),SelectedSlot==2);
        Send(EKeys::Down);Send(EKeys::Enter);
        Check(TEXT("IFileManager::Get().FileExists(*P->State()->SlotPath(2))"),IFileManager::Get().FileExists(*P->State()->SlotPath(2)));
        const auto Id=P->State()->State.Companions[0].IndividualId;
        P->State()->State.PlayerName=TEXT("temporary test");
        Send(EKeys::Enter);Check(TEXT("PendingAction==2"),PendingAction==2);
        Send(EKeys::Escape);Check(TEXT("PendingAction==-1&&IsMenuOpen()"),PendingAction==-1&&IsMenuOpen());
        Send(EKeys::Down);Send(EKeys::Enter);Check(TEXT("PendingAction==3"),PendingAction==3);
        Send(EKeys::Enter);Check(TEXT("P->State()->State.PlayerName!=TEXT(\"temporary test\")&&P->State()->State.Companions[0].IndividualId==Id"),P->State()->State.PlayerName!=TEXT("temporary test")&&P->State()->State.Companions[0].IndividualId==Id);
        P->State()->WritesBlocked=true;
        Check(TEXT("!P->State()->LoadSlot(3)&&P->State()->WritesBlocked"),!P->State()->LoadSlot(3)&&P->State()->WritesBlocked);
        Check(TEXT("P->State()->LoadSlot(2)&&!P->State()->WritesBlocked"),P->State()->LoadSlot(2)&&!P->State()->WritesBlocked);
        const bool Story=P->State()->State.StoryDifficulty;ActivateMenuRow(5);
        Check(TEXT("P->State()->State.StoryDifficulty!=Story"),P->State()->State.StoryDifficulty!=Story);
        ActivateMenuRow(4);Check(TEXT("PendingAction==4"),PendingAction==4);
        ActivateMenuRow(4);Check(TEXT("P->State()->State.StoryDifficulty!=Story"),P->State()->State.StoryDifficulty!=Story);
        ActivateMenuRow(8);Check(TEXT("PendingAction==8"),PendingAction==8);
        Send(EKeys::Gamepad_FaceButton_Right);Check(TEXT("PendingAction==-1&&IsMenuOpen()"),PendingAction==-1&&IsMenuOpen());
        ActivateMenuRow(6);ActivateMenuRow(1);float Stored=-1;FConfigFile SavedSettings;SavedSettings.Read(SettingsPath());SavedSettings.GetFloat(TEXT("Audio"),TEXT("MasterVolume"),Stored);Check(TEXT("FMath::IsNearlyEqual(Stored,.9f)"),FMath::IsNearlyEqual(Stored,.9f));
        ActivateMenuRow(2);ActivateMenuRow(3);ActivateMenuRow(4);
        LoadPresentationSettings();Check(TEXT("presentation settings reopen"),FMath::IsNearlyEqual(MusicVolume,.9f)&&FMath::IsNearlyEqual(EffectsVolume,.9f)&&DialogueFontSize==24);
        for(int32 N=0;N<9;++N)ActivateMenuRow(2);
        for(TActorIterator<AVZAudioDirector> It(GetWorld());It;++It)Check(TEXT("music mute applies while paused"),It->Calm->VolumeMultiplier==0 && It->Alert->VolumeMultiplier==0 && It->Exploration->VolumeMultiplier==0);
        Check(TEXT("effects independent of music mute"),FMath::IsNearlyEqual(EffectsVolume,.9f));
        ActivateMenuRow(2);Check(TEXT("music volume wraps after zero"),MusicVolume==1);
        MenuBack();Check(TEXT("presentation back keeps pause"),!PresentationPage&&IsMenuOpen());
        ActivateMenuRow(7);BindingIndex=9;ActivateMenuRow(2);
        Check(TEXT("reject conflicting binding"),!CaptureBinding(EKeys::W)&&CaptureDevice==0);
        Check(TEXT("capture and apply key"),CaptureBinding(EKeys::F)&&CaptureDevice==-1);
        Check(TEXT("live input mapping replaced"),PlayerInput->ActionMappings.ContainsByPredicate([](const FInputActionKeyMapping& M){return M.ActionName==TEXT("VZPulse")&&M.Key==EKeys::F;})&&!PlayerInput->ActionMappings.ContainsByPredicate([](const FInputActionKeyMapping& M){return M.ActionName==TEXT("VZPulse")&&M.Key==EKeys::LeftMouseButton;}));
        FVZInputSettings Reopened;Check(TEXT("reopen persisted bindings"),Reopened.Load(SettingsPath())&&Reopened.Keyboard[9]==EKeys::F);
        LoadPresentationSettings();Check(TEXT("binding save retains accessibility"),DialogueFontSize==24 && FMath::IsNearlyEqual(EffectsVolume,.9f));
        MenuBack();
        ActivateMenuRow(6);ActivateMenuRow(5);
        Check(TEXT("reset waits for second confirmation"),PendingAction==5 && DialogueFontSize==24);
        MenuBack();Check(TEXT("cancel reset preserves settings"),PresentationPage && DialogueFontSize==24);
        ActivateMenuRow(5);ActivateMenuRow(5);
        Check(TEXT("reset preserves bindings"),MasterVolume==1 && MusicVolume==1 && EffectsVolume==1 && DialogueFontSize==18 && Bindings.Keyboard[9]==EKeys::F);
        FConfigFile Invalid;Invalid.Read(SettingsPath());Invalid.SetFloat(TEXT("Audio"),TEXT("MusicVolume"),9);Invalid.SetFloat(TEXT("Audio"),TEXT("EffectsVolume"),-2);Invalid.SetString(TEXT("Accessibility"),TEXT("DialogueFontSize"),TEXT("999"));Invalid.Write(SettingsPath());
        LoadPresentationSettings();Check(TEXT("invalid settings clamped"),MusicVolume==1 && EffectsVolume==0 && DialogueFontSize==18);
        EffectsVolume=.9f;DialogueFontSize=24;SavePresentationSettings();ApplyVolume();
        FScreenshotRequest::RequestScreenshot(TEXT("VariantZero_Presentation.png"),true,false);
        VerificationStep=2;
    }
    else if(VerificationStep==2&&Elapsed>4)
    {
        MenuBack();
        auto KeepAlive=Menu;Menu->OnKeyDown(FGeometry(),FKeyEvent(EKeys::Gamepad_FaceButton_Right,FModifierKeysState(),0,false,0,0));
        Check(TEXT("!IsMenuOpen()&&!UGameplayStatics::IsGamePaused(this)&&!P->PulseHeld&&!bShowMouseCursor"),!IsMenuOpen()&&!UGameplayStatics::IsGamePaused(this)&&!P->PulseHeld&&!bShowMouseCursor);
        P->Prologue->Show(TEXT("字号检查"),TEXT("测试正文：让不同生命共同生长。"));
        bool FontApplied=false;
        for(TObjectIterator<UVZDialogueWidget> It;It;++It)if(It->GetWorld()==GetWorld() && It->IsInViewport())
        {
            TArray<UWidget*> TextWidgets;It->WidgetTree->GetAllWidgets(TextWidgets);
            for(auto* Widget:TextWidgets)if(auto* Text=Cast<UTextBlock>(Widget))
                if(Text->GetText().ToString()==TEXT("测试正文：让不同生命共同生长。"))FontApplied=Text->GetFont().Size==24;
        }
        Check(TEXT("actual dialogue widget uses saved font"),FontApplied);P->Prologue->Dismiss();
        // Exercise the real pause UI and persistence path with isolated test saves.
        if(P->State()->State.Party.Num()<2)
        {
            auto Extra=P->State()->State.Companions[0];Extra.IndividualId=FGuid::NewGuid();Extra.Name=TEXT("测试伙伴");Extra.Memories.Reset();
            P->State()->State.Companions.Add(Extra);P->State()->State.Party.Add(Extra.IndividualId);P->RebuildParty();
        }
        ToggleMenu();ActivateMenuRow(9);ActivateMenuRow(5);RosterIndex=0;SelectCompanion(0);
        Check(TEXT("roster accessible from pause"),RosterPage);
        const FGuid Original=P->State()->State.Companions[0].IndividualId;
        RenameDraft=TEXT("雨芽");ActivateMenuRow(3);
        Check(TEXT("rename applied to selected identity"),P->State()->State.Companions[0].Name==TEXT("雨芽"));
        P->State()->WritesBlocked=true;RenameDraft=TEXT("不应提交");ActivateMenuRow(3);P->State()->WritesBlocked=false;
        Check(TEXT("failed save preserves name"),P->State()->State.Companions[0].Name==TEXT("雨芽"));
        auto* SameActor=P->Companions[0].Get();SameActor->Vital->Health=73;P->CompanionSkillCooldown=4;P->CompanionSkillCooldown2=1;
        ActivateMenuRow(5);
        Check(TEXT("reorder keeps actor health cooldown and identity"),P->State()->State.Party[1]==Original&&P->Companions[1]==SameActor&&SameActor->Vital->Health==73&&P->CompanionSkillCooldown2==4&&P->CompanionSkillCooldown==1);
        FVZWorldState SavedRoster;Check(TEXT("roster changes persisted"),FVZSaveStore::Read(P->State()->SlotPath(0),SavedRoster)==EVZReadResult::Ok&&SavedRoster.Party[1]==Original&&SavedRoster.Companions[0].Name==TEXT("雨芽"));
        ActivateMenuRow(4);SameActor->Vital->Restore();P->CompanionSkillCooldown=0;P->CompanionSkillCooldown2=0;
        const FVector BeforeRest=P->GetActorLocation();
        P->SetActorLocation(FVector(0,3000,120));P->Vital->Health=70;
        Check(TEXT("remote roster change cannot heal"),!ToggleCompanionDeployment()&&P->Vital->Health==70&&P->State()->State.Party.Contains(Original));
        P->SetActorLocation(FVector(-650,350,120));
        P->Companions[0]->HasGroundOrder=true;
        Check(TEXT("active companion order blocks rest"),!ToggleCompanionDeployment());P->Companions[0]->HasGroundOrder=false;
        P->State()->WritesBlocked=true;
        Check(TEXT("failed roster save preserves actors and health"),!ToggleCompanionDeployment()&&P->Companions[0]==SameActor&&P->Vital->Health==70&&P->State()->State.Party.Contains(Original));
        P->State()->WritesBlocked=false;
        ActivateMenuRow(6);
        Check(TEXT("reserve removes active AI and heals at station"),P->State()->State.Party.Num()==1&&!P->State()->State.Party.Contains(Original)&&P->Companions.Num()==1&&!IsValid(SameActor)&&P->Vital->Health==P->Vital->MaxHealth);
        RosterIndex=1;SelectCompanion(0);
        Check(TEXT("cannot bench last companion"),!ToggleCompanionDeployment());
        RosterIndex=0;SelectCompanion(0);ActivateMenuRow(6);
        Check(TEXT("original individual can rejoin"),P->Companions.Num()==2&&P->Companions[1]->IndividualId==Original&&P->State()->State.Companions[0].Name==TEXT("雨芽"));
        Check(TEXT("read roster after rest"),FVZSaveStore::Read(P->State()->SlotPath(0),SavedRoster)==EVZReadResult::Ok&&SavedRoster.Party[1]==Original);
        const auto BeforeGrowth=P->State()->State;
        auto GrowthFixture=BeforeGrowth;GrowthFixture.QuestStages.Add(TEXT("main.prologue"),6);GrowthFixture.QuestStages.Add(TEXT("main.garden"),3);
        Check(TEXT("growth fixture"),P->State()->Commit(GrowthFixture));
        RosterIndex=0;SelectCompanion(0);ActivateMenuRow(7);
        Check(TEXT("growth requires explicit second click"),PendingAction==101&&P->State()->State.Companions[0].Level==1);
        P->State()->WritesBlocked=true;ActivateMenuRow(7);
        Check(TEXT("growth failed save rolls back points and level"),P->State()->State.Companions[0].Level==1&&FVZRosterRules::GrowthPoints(P->State()->State)==8);
        P->State()->WritesBlocked=false;ActivateMenuRow(7);ActivateMenuRow(7);
        Check(TEXT("growth persisted and no duplicate stale upgrade"),P->State()->State.Companions[0].Level==2&&FVZRosterRules::GrowthPoints(P->State()->State)==7&&!TrainCompanion(1));
        bool LiveGrowth=false;for(auto C:P->Companions)if(C->IndividualId==Original)LiveGrowth=C->Vital->MaxHealth==105&&C->Vital->Health==105;
        Check(TEXT("growth applies real companion health"),LiveGrowth);
        Check(TEXT("growth save read"),FVZSaveStore::Read(P->State()->SlotPath(0),SavedRoster)==EVZReadResult::Ok&&SavedRoster.Companions[0].Level==2&&FVZRosterRules::GrowthPoints(SavedRoster)==7);
        Check(TEXT("restore pre-growth fixture"),P->State()->Commit(BeforeGrowth));P->RebuildParty();
        ActivateMenuRow(4);P->SetActorLocation(BeforeRest);
        Check(TEXT("growth cannot be performed remotely"),!TrainCompanion(1));
        MenuBack();Check(TEXT("roster back returns to archive"),!RosterPage&&ArchivePage);CloseMenu();
        P->StartEncounter();P->Scan();PausedAttackClock=P->Disturbance->AttackClock;
        PlayerInput->InputKey(FInputKeyParams(EKeys::F,IE_Pressed,1.0));VerificationStep=3;
    }
    else if(VerificationStep==3&&Elapsed>5)
    {
        Check(TEXT("P->Disturbance->AttackClock<PausedAttackClock"),P->Disturbance->AttackClock<PausedAttackClock);
        Check(TEXT("rebound key dispatches gameplay"),P->PulseHeld&&P->Disturbance->Vital->Health<=168);
        PlayerInput->InputKey(FInputKeyParams(EKeys::F,IE_Released,0.0));
        bool AudioReady=false;for(TActorIterator<AVZAudioDirector> It(GetWorld());It;++It)AudioReady=It->Calm->Sound&&It->Alert->Sound&&It->Pulse&&It->Shield&&It->Mix>.2f;
        Check(TEXT("offline audio loaded and battle mix transitions"),AudioReady);
        RenameDraft=TEXT("战斗中改名");Check(TEXT("combat rejects roster edits"),!EditCompanion(true)&&P->State()->State.Companions[0].Name==TEXT("雨芽"));
        const FVector CombatPosition=P->GetActorLocation();P->SetActorLocation(FVector(-650,350,120));
        Check(TEXT("combat blocks institute deployment"),!ToggleCompanionDeployment());P->SetActorLocation(CombatPosition);
        P->SetActorLocation(FVector(-650,350,120));Check(TEXT("combat blocks training"),!TrainCompanion(1));P->SetActorLocation(CombatPosition);
        for(TActorIterator<AVZAudioDirector> It(GetWorld());It;++It)
        {
            Check(TEXT("exploration loop cooked and playing"),It->Exploration->Sound&&It->Exploration->IsPlaying());
            const float OldMix=It->Mix,OldRegion=It->RegionMix;
            It->Mix=0;It->RegionMix=1;It->ApplyMix();
            Check(TEXT("garden calm routing"),It->Calm->VolumeMultiplier==0&&It->Exploration->VolumeMultiplier>0&&It->Alert->VolumeMultiplier==0);
            It->Mix=1;It->ApplyMix();
            Check(TEXT("combat suppresses birds and vocals"),It->Exploration->VolumeMultiplier==0&&It->Alert->VolumeMultiplier>0);
            It->RegionMix=0;It->Mix=0;It->ApplyMix();
            Check(TEXT("institute routing"),It->Calm->VolumeMultiplier>0&&It->Exploration->VolumeMultiplier==0);
            It->LastHitTime=-1;P->Vital->Restore();
            Check(TEXT("damage triggers hit cue"),P->Vital->ReceiveDamage(1)>0&&It->LastHitTime>=0);
            const double HitStamp=It->LastHitTime;P->Vital->InvulnerableSeconds=1;
            Check(TEXT("invulnerability does not emit hit cue"),P->Vital->ReceiveDamage(1)==0&&It->LastHitTime==HitStamp);
            P->Vital->Restore();It->Mix=OldMix;It->RegionMix=OldRegion;It->ApplyMix();
        }
        UE_LOG(LogTemp,Display,TEXT("VZ_MENU: %s pause freezes combat, keyboard/gamepad navigation, save/load confirmation, identity, difficulty, volume, resume"),VerificationOk?TEXT("PASS"):TEXT("FAIL"));
        if(!VerificationOk){FPlatformMisc::RequestExitWithStatus(true,1);return;}
        ToggleMenu();ActivateMenuRow(9);ActivateMenuRow(5);RosterIndex=0;SelectCompanion(0);
        FScreenshotRequest::RequestScreenshot(TEXT("VariantZero_Roster.png"),true,false);
        VerificationStep=4;
    }
    else if(VerificationStep==4&&Elapsed>7)FPlatformMisc::RequestExitWithStatus(true,VerificationOk?0:1);
#endif
}
