#include "VZPrologue.h"
#include "VZPrototype.h"
#include "VZCombat.h"
#include "VZAudioDirector.h"
#include "VZPlayerController.h"
#include "VZGarden.h"
#include "VZChuyaStory.h"
#include "VZPlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"
#include "Components/VerticalBox.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/StaticMeshComponent.h"
#include "Components/TextRenderComponent.h"
#include "Engine/StaticMeshActor.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"

namespace
{
const FString Quest=TEXT("main.prologue");
const FVector Terminal(-650,-250,90), Nursery(-650,350,90), Flower(650,0,90);
const FVector Samples[]={FVector(-900,-720,95),FVector(0,900,95),FVector(980,620,95)};
FString SampleId(int32 I) { return FString::Printf(TEXT("prologue.sample.%d"),I); }
}
int32 FVZPrologueRules::Stage(const FVZWorldState& S) { return S.QuestStages.FindRef(Quest); }
bool FVZPrologueRules::Apply(FVZWorldState& S, const FString& Event)
{
    if (!FVZSaveStore::Validate(S)) return false;
    auto C=S;
    const int32 Before=Stage(C);
    const FString Receipt=TEXT("prologue.event.")+Event+TEXT(".v1");
    if(C.RewardTransactions.Contains(Receipt)) return false;
    if(Event==TEXT("briefing") && Before==0) C.QuestStages.Add(Quest,1);
    else if(Event==TEXT("scan") && Before==1) C.QuestStages.Add(Quest,2);
    else if(Event.StartsWith(TEXT("sample.")) && Before==2)
    {
        int32 Index=INDEX_NONE;
        for(int32 I=0;I<3;++I) if(Event==FString::Printf(TEXT("sample.%d"),I)) Index=I;
        if(Index==INDEX_NONE || C.Repairs.Contains(SampleId(Index))) return false;
        C.Repairs.Add(SampleId(Index));
        C.Materials.FindOrAdd(TEXT("prologue.viable_sample"))++;
        if(C.Repairs.Contains(SampleId(0)) && C.Repairs.Contains(SampleId(1)) && C.Repairs.Contains(SampleId(2))) C.QuestStages.Add(Quest,3);
    }
    else if(Event==TEXT("cultivate") && Before==3)
    {
        auto* Count=C.Materials.Find(TEXT("prologue.viable_sample"));
        if(!Count || *Count<3 || C.Companions.Num()>=512) return false;
        *Count-=3;
        FVZCompanion Child; Child.IndividualId=FGuid::NewGuid();Child.SpeciesId=TEXT("V-001");Child.Name=TEXT("新芽");Child.Memories.Add(TEXT("prologue.first_cultivation"));
        C.Companions.Add(Child);
        if(C.Party.Num()<3) C.Party.Add(Child.IndividualId);
        C.QuestStages.Add(Quest,4);
    }
    else if(Event==TEXT("practice") && Before==4) C.QuestStages.Add(Quest,5);
    else if(Event==TEXT("repair") && Before==5)
    {
        C.Repairs.Add(TEXT("prologue.flowerbed"));
        C.Companions[0].Memories.AddUnique(TEXT("prologue.first_flower"));
        C.Materials.FindOrAdd(TEXT("research.sample"))+=2;
        C.QuestStages.Add(Quest,6);
    }
    else if(Event==TEXT("debrief") && Before==6)
    {
        C.QuestStages.Add(Quest,7);
        C.DrillUnlocks.Add(TEXT("V-006"));
    }
    else if(Event==TEXT("research_moth") && Before==7)
    {
        auto* Count=C.Materials.Find(TEXT("research.sample"));
        if(!Count || *Count<2 || C.Companions.Num()>=512 || C.Party.Num()>=3)return false;
        *Count-=2;
        FVZCompanion Moth;Moth.IndividualId=FGuid::NewGuid();Moth.SpeciesId=TEXT("V-041");Moth.Name=TEXT("菌翼蛾");Moth.Memories.Add(TEXT("prologue.pollinator_research"));
        C.Companions.Add(Moth);C.Party.Add(Moth.IndividualId);C.DrillUnlocks.Add(TEXT("V-041"));
    }
    else if(Event==TEXT("pollinate") && Before==7)
    {
        bool InParty=false;for(const auto& Individual:C.Companions)if(Individual.SpeciesId==TEXT("V-041") && C.Party.Contains(Individual.IndividualId))InParty=true;
        if(!InParty)return false;
        C.Repairs.Add(TEXT("prologue.pollinated"));
        for(auto& Individual:C.Companions)if(Individual.SpeciesId==TEXT("V-041") && C.Party.Contains(Individual.IndividualId))Individual.Memories.AddUnique(TEXT("prologue.first_pollination"));
    }
    else return false;
    C.RewardTransactions.Add(Receipt);
    if(!FVZSaveStore::Validate(C)) return false;
    S=MoveTemp(C);return true;
}
FString FVZPrologueRules::Objective(const FVZWorldState& S)
{
    switch(Stage(S))
    {
    case 0:return TEXT("温室留言 · 靠近西侧档案台交互");
    case 1:return TEXT("不再结果的花 · 靠近中央花床扫描");
    case 2:return FString::Printf(TEXT("保留差异 · 收集三处活性样本（%d / 3）"),S.Materials.FindRef(TEXT("prologue.viable_sample")));
    case 3:return TEXT("第一次培育 · 返回西北培养台，消耗 3 份样本");
    case 4:return TEXT("一起练习 · 靠近南侧节点交互，协同完成练习");
    case 5:return TEXT("允许生长 · 返回花床交互，恢复供养");
    case 6:return TEXT("回信 · 返回档案台提交观察记录");
    default:return !S.RewardTransactions.Contains(TEXT("prologue.event.research_moth.v1"))?TEXT("传粉研究 · 培养台消耗 2 份研究样本培育菌翼蛾"):!S.Repairs.Contains(TEXT("prologue.pollinated"))?TEXT("带菌翼蛾回花床交互，恢复传粉"):TEXT("传粉完成 · 档案台可回看记录");
    }
}
FString FVZPrologueRules::Journal(const FVZWorldState& S)
{
    FString Text=TEXT("【温室留言】\n管理员：你到的时候，花应该还开着。别让这句话骗了你——这里已经七年没有结过一颗种子。先带初芽去看看。\n");
    if(Stage(S)>=2) Text+=TEXT("\n【扫描记录】\n叶脉正在重复同一段春天。它不是缺少营养，而是无法回应今天的光。三处边缘植株仍保留不同的反应。\n");
    if(Stage(S)>=4) Text+=TEXT("\n【培育记录】\n我们保留了三个样本的差异。新芽不是初芽的替代品，也不需要牺牲它的母体。它们会一起生活。\n");
    if(Stage(S)>=6) Text+=TEXT("\n【第一株花】\n供养恢复了。初芽在花床旁停了很久。我们还不知道它会结出什么，只知道这一次，结果不必和过去一样。\n");
    if(Stage(S)>=7) Text+=TEXT("\n【来自花庭】\n管理员：无访花庭曾经有成千上万的传粉者。现在那些花还在等。带上今天的记录；也许外面的世界，正在等一个不同的答案。\n\n花庭开放了机关与修复原型。本流程尚未达到 30–45 分钟正式样章的验收标准。");
    if(S.Repairs.Contains(TEXT("prologue.pollinated")))Text+=TEXT("\n\n【传粉记录】\n菌翼蛾停在花冠上，带来了花粉。供养让花活着，生命之间的来往让它拥有明天。");
    return Text;
}
TArray<TPair<FVector,FString>> FVZPrologueRules::Targets(const FVZWorldState& S)
{
    TArray<TPair<FVector,FString>> Result;
    switch(Stage(S))
    {
    case 0:case 6:Result.Emplace(Terminal,TEXT("档案台"));break;
    case 7:
        if(!S.RewardTransactions.Contains(TEXT("prologue.event.research_moth.v1")))Result.Emplace(Nursery,TEXT("传粉研究"));
        else if(!S.Repairs.Contains(TEXT("prologue.pollinated")))Result.Emplace(Flower,TEXT("恢复传粉"));
        else Result.Emplace(Terminal,TEXT("档案台"));break;
    case 1:case 5:Result.Emplace(Flower,TEXT("花床"));break;
    case 2:for(int32 I=0;I<3;++I)if(!S.Repairs.Contains(SampleId(I)))Result.Emplace(Samples[I],TEXT("活性样本"));break;
    case 3:Result.Emplace(Nursery,TEXT("培养台"));break;
    case 4:Result.Emplace(FVector(550,-450,130),TEXT("协作练习"));break;
    default:break;
    }
    return Result;
}
UVZPrologue::UVZPrologue() { PrimaryComponentTick.bCanEverTick=true; }
AVZEcologist* UVZPrologue::Player() const { return Cast<AVZEcologist>(GetOwner()); }
bool UVZPrologue::Enabled() const { return FParse::Param(FCommandLine::Get(),TEXT("VZChuyaStoryTest")) || FParse::Param(FCommandLine::Get(),TEXT("VZStory")) || FParse::Param(FCommandLine::Get(),TEXT("VZStoryTest")) || FParse::Param(FCommandLine::Get(),TEXT("VZGardenTest")) || FParse::Param(FCommandLine::Get(),TEXT("VZTravelTest")); }
void UVZPrologue::BeginPlay() { Super::BeginPlay();if(Enabled()) Refresh(); }
bool UVZPrologue::Apply(const FString& Event)
{
    auto* P=Player();if(!P)return false;
    auto C=P->State()->State;
    if(!FVZPrologueRules::Apply(C,Event))return false;
    if(!P->State()->Commit(C))return false;
    Refresh();return true;
}
FString UVZPrologue::Objective() const
{
    if(!Player())return FString();
    if(Player()->ChuyaStory->Enabled())return FVZChuyaStoryRules::Objective(Player()->State()->State);
    if(Player()->Garden->Enabled() && Player()->State()->State.Repairs.Contains(TEXT("prologue.pollinated")))return FVZGardenRules::Objective(Player()->State()->State);
    return FVZPrologueRules::Objective(Player()->State()->State);
}
void UVZPrologue::Refresh()
{
    CancelPollination();
    if(!Enabled() || !Player())return;
    for(auto A:Markers) if(IsValid(A))A->Destroy();Markers.Reset();
    const auto& S=Player()->State()->State;
    const int32 Stage=FVZPrologueRules::Stage(S);
    auto Marker=[&](FVector Pos,const TCHAR* Label)
    {
        auto* A=GetWorld()->SpawnActor<AStaticMeshActor>(Pos-FVector(0,0,50),FRotator::ZeroRotator);
        A->GetStaticMeshComponent()->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
        A->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        A->GetStaticMeshComponent()->SetCanEverAffectNavigation(false);
        A->SetActorScale3D(FVector(.6,.6,.65));Markers.Add(A);
        auto* Text=NewObject<UTextRenderComponent>(A);Text->SetupAttachment(A->GetRootComponent());Text->RegisterComponent();
        Text->SetText(FText::FromString(Label));Text->SetWorldSize(22);Text->SetWorldScale3D(FVector(1));Text->SetWorldLocation(Pos+FVector(0,0,100));Text->SetHorizontalAlignment(EHTA_Center);Text->SetTextRenderColor(FColor(155,255,210));
    };
    Marker(Terminal,TEXT("ARCHIVE"));Marker(Nursery,TEXT("NURSERY"));
    if(Stage==2)for(int32 I=0;I<3;++I)if(!S.Repairs.Contains(SampleId(I)))Marker(Samples[I],TEXT("SAMPLE"));
    Player()->RefreshEcology();
}
void UVZPrologue::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Fn)
{
    Super::TickComponent(Dt,Type,Fn);if(!Enabled() || !Player())return;
    if(Pollinator.IsValid())
    {
        auto* Moth=Pollinator.Get();
        if(Moth->Vital->IsDown() || FVector::Dist2D(Player()->GetActorLocation(),Flower)>500 || Player()->Disturbance->Active)
        {CancelPollination();Player()->State()->LastMessage=TEXT("传粉已中断，没有结算；可返回花床重试。");}
        else
        {
            Moth->FormationOffset=Player()->GetActorRotation().UnrotateVector(Flower-Player()->GetActorLocation());
            PollinationSeconds=FVector::Dist2D(Moth->GetActorLocation(),Flower)<120?PollinationSeconds+Dt:0;
            if(PollinationSeconds>=1.4f)
            {
                CancelPollination();
                if(Apply(TEXT("pollinate"))){AVZAudioDirector::PlayCue(GetWorld(),TEXT("Repair"));Show(TEXT("传粉完成"),TEXT("菌翼蛾停在花冠上，带来了花粉。共同经历和花床状态已一起保存。"));}
            }
        }
    }
    if(FVZPrologueRules::Stage(Player()->State()->State)==4 && IsValid(Player()->Disturbance) && Player()->Disturbance->Cleared)
        if(Apply(TEXT("practice"))) Player()->State()->LastMessage=TEXT("协作练习完成。回到花床，让供养重新流动。");
}
void UVZPrologue::CancelPollination()
{
    if(Pollinator.IsValid())Pollinator->FormationOffset=PreviousFormation;
    Pollinator.Reset();PollinationSeconds=0;
}
bool UVZPrologue::Scan()
{
    if(!Enabled())return false;
    if(FVZPrologueRules::Stage(Player()->State()->State)!=1)return false;
    if(FVector::Dist(Player()->GetActorLocation(),Flower)>400){Player()->State()->LastMessage=TEXT("靠近中央花床后扫描。");return true;}
    if(Apply(TEXT("scan")))Show(TEXT("扫描记录 · 不再结果的花"),TEXT("营养充足，传粉信号缺失。叶脉正在重复一段过时的环境记忆。\n\n边缘的三处植株保留了不同反应。去采集样本，不必拔走它们。"));
    return true;
}
bool UVZPrologue::Interact()
{
    if(!Enabled())return false;
    if(Dialogue){Dismiss();return true;}
    auto* P=Player();const auto Pos=P->GetActorLocation();const int32 Stage=FVZPrologueRules::Stage(P->State()->State);
    if(FVector::Dist(Pos,Terminal)<220)
    {
        if(Stage==0 && !Apply(TEXT("briefing")))return true;
        if(Stage==6 && !Apply(TEXT("debrief")))return true;
        Show(TEXT("原初研究所 · 档案记录"),FVZPrologueRules::Journal(P->State()->State));return true;
    }
    if(Stage==2)for(int32 I=0;I<3;++I)if(FVector::Dist(Pos,Samples[I])<180)
    {
        if(Apply(FString::Printf(TEXT("sample.%d"),I)))P->State()->LastMessage=TEXT("已保存样本与采集位置；同一植株不会重复提供样本。");
        else P->State()->LastMessage=TEXT("这株植物的样本已经保存。");
        return true;
    }
    if(FVector::Dist(Pos,Nursery)<220)
    {
        if(Stage==7)
        {
            if(Apply(TEXT("research_moth"))){P->RebuildParty();Show(TEXT("菌翼蛾 · 传粉伙伴"),TEXT("消耗 2 份研究样本，培育菌翼蛾并加入队伍。原有伙伴全部保留。\n\n带它靠近花床并交互，恢复传粉。"));}
            else P->State()->LastMessage=TEXT("研究已完成，或样本不足／队伍已满。没有扣除材料。");
            return true;
        }
        if(Stage==3 && Apply(TEXT("cultivate")))
        {P->RebuildParty();Show(TEXT("确定性培育 · 新芽"),TEXT("消耗 3 份活性样本，获得原豆母体个体「新芽」。\n\n首次培育必定成功。初芽被保留，新伙伴已加入队伍。\n\n现在去南侧节点一起练习；这是研究所的训练装置。"));}
        else P->State()->LastMessage=TEXT("培养台：完成扫描并收齐三处活性样本后可进行首次培育。");
        return true;
    }
    if(Stage==4 && IsValid(P->Disturbance) && FVector::Dist(Pos,P->Disturbance->GetActorLocation())<260)
    {P->StartEncounter();return true;}
    if(FVector::Dist(Pos,Flower)<240)
    {
        if(Stage==7)
        {
            AVZCompanionProxy* Moth=nullptr;for(auto C:P->Companions)if(IsValid(C) && C->SpeciesId==TEXT("V-041"))Moth=C;
            if(!Moth || Moth->Vital->IsDown() || FVector::Dist2D(Moth->GetActorLocation(),Flower)>450){P->State()->LastMessage=TEXT("需要健康的菌翼蛾跟到花床旁，才能传粉。");return true;}
            if(P->State()->State.Repairs.Contains(TEXT("prologue.pollinated"))){P->State()->LastMessage=TEXT("花床已经恢复传粉。");return true;}
            if(!Pollinator.IsValid()){Pollinator=Moth;PreviousFormation=Moth->FormationOffset;Moth->CancelOrder();PollinationSeconds=0;}
            P->State()->LastMessage=TEXT("菌翼蛾正在前往花床。留在附近，等待它完成传粉。");
            return true;
        }
        if(Stage==5 && Apply(TEXT("repair"))) {AVZAudioDirector::PlayCue(GetWorld(),TEXT("Repair"));Show(TEXT("第一株花"),TEXT("初芽停在花床边，新芽靠近了它。供养恢复了。\n\n我们还不知道它会结出什么。把这段观察带回档案台。\n\n获得 2 份研究样本；记录与奖励已一起保存。"));}
        else P->State()->LastMessage=Objective();
        return true;
    }
    P->State()->LastMessage=Objective();return true;
}
void UVZPrologue::Show(const FString& Title,const FString& Text)
{
    if(IsValid(Player()->Disturbance) && Player()->Disturbance->Active){Player()->State()->LastMessage=TEXT("先完成或离开协作练习，再查阅记录。");return;}
    Dismiss();auto* PC=Cast<APlayerController>(Player()->GetController());if(!PC)return;
    Dialogue=CreateWidget<UVZDialogueWidget>(PC,UVZDialogueWidget::StaticClass());
    auto* Tree=Dialogue->WidgetTree.Get();
    auto* Root=Tree->ConstructWidget<UCanvasPanel>();Tree->RootWidget=Root;
    auto* Border=Tree->ConstructWidget<UBorder>();Border->SetBrushColor(FLinearColor(.012,.035,.028,.98));Border->SetPadding(FMargin(28));
    auto* Slot=Root->AddChildToCanvas(Border);Slot->SetAnchors(FAnchors(.12f,.18f,.88f,.8f));Slot->SetOffsets(FMargin(0));
    auto* Scroll=Tree->ConstructWidget<UScrollBox>();Border->SetContent(Scroll);
    auto* Column=Tree->ConstructWidget<UVerticalBox>();Scroll->AddChild(Column);
    auto AddText=[&](const FString& Value,int32 Size,FLinearColor Color)
    {auto* T=Tree->ConstructWidget<UTextBlock>();T->SetText(FText::FromString(Value));T->SetAutoWrapText(true);auto Font=T->GetFont();Font.Size=Size;T->SetFont(Font);T->SetColorAndOpacity(FSlateColor(Color));Column->AddChildToVerticalBox(T);};
    const auto* Settings=Cast<AVZPlayerController>(PC);const int32 TextSize=Settings?Settings->DialogueFontSize:18;
    AddText(Title+TEXT("\n"),TextSize+6,FLinearColor(.55,1,.77));AddText(Text,TextSize,FLinearColor(.92,.94,.85));
    auto* Button=Tree->ConstructWidget<UButton>();auto* Label=Tree->ConstructWidget<UTextBlock>();Label->SetText(FText::FromString(TEXT("继续探索（交互键 / 点击）")));Button->SetContent(Label);Column->AddChildToVerticalBox(Button);Button->OnClicked.AddDynamic(this,&UVZPrologue::Dismiss);
    Dialogue->AddToViewport(30);Player()->StopPulse();Player()->GetCharacterMovement()->StopMovementImmediately();
    PC->SetIgnoreMoveInput(true);PC->SetIgnoreLookInput(true);PC->bShowMouseCursor=true;
    FInputModeGameAndUI Input;Input.SetHideCursorDuringCapture(false);PC->SetInputMode(Input);
}
void UVZPrologue::Dismiss()
{
    if(!Dialogue)return;Dialogue->RemoveFromParent();Dialogue=nullptr;
    if(auto* PC=Cast<APlayerController>(Player()->GetController()))
    {PC->SetIgnoreMoveInput(false);PC->SetIgnoreLookInput(false);PC->bShowMouseCursor=false;PC->SetInputMode(FInputModeGameOnly());}
}
