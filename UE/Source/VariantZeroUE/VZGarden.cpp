#include "VZGarden.h"
#include "VZPrototype.h"
#include "VZPrologue.h"
#include "VZChuyaStory.h"
#include "VZAudioDirector.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "DrawDebugHelpers.h"
#include "EngineUtils.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
const FVector Entrance(0,1100,90),Archive(0,2500,90),Flower(0,5100,90);
const FVector Mirrors[]={FVector(-750,4250,90),FVector(750,4250,90),FVector(0,6500,90)};
const int32 Correct[]={1,3,2};
FString Key(int32 I){return FString::Printf(TEXT("garden.mirror.%d"),I);}
bool HasMoth(const FVZWorldState& S){for(const auto& C:S.Companions)if(C.SpeciesId==TEXT("V-041") && S.Party.Contains(C.IndividualId))return true;return false;}
}
int32 FVZGardenRules::Stage(const FVZWorldState& S){return S.QuestStages.FindRef(TEXT("main.garden"));}
bool FVZGardenRules::Aligned(const FVZWorldState& S){for(int32 I=0;I<3;++I)if(S.QuestStages.FindRef(Key(I))!=Correct[I])return false;return true;}
bool FVZGardenRules::Apply(FVZWorldState& S,const FString& Event)
{
    if(!FVZSaveStore::Validate(S))return false;
    auto C=S;const int32 Before=Stage(C);
    if(Event==TEXT("enter") && Before==0 && C.Repairs.Contains(TEXT("prologue.pollinated")) && HasMoth(C))
    {C.QuestStages.Add(TEXT("main.garden"),1);C.Checkpoint=FVector(0,2200,120);}
    else if(Event.StartsWith(TEXT("mirror.")) && Before==1)
    {
        int32 Index=INDEX_NONE;for(int32 I=0;I<3;++I)if(Event==FString::Printf(TEXT("mirror.%d"),I))Index=I;
        if(Index==INDEX_NONE)return false;
        C.QuestStages.FindOrAdd(Key(Index))=(C.QuestStages.FindRef(Key(Index))+1)%4;
    }
    else if(Event==TEXT("repair") && Before==1 && Aligned(C) && HasMoth(C))
    {
        const FString Receipt=TEXT("garden.repair.v1");if(C.RewardTransactions.Contains(Receipt))return false;
        C.RewardTransactions.Add(Receipt);C.Repairs.Add(TEXT("garden.waterway"));C.Repairs.Add(TEXT("garden.shortcut"));C.Materials.FindOrAdd(TEXT("research.sample"))+=4;
        C.QuestStages.Add(TEXT("main.garden"),2);
        for(auto& Partner:C.Companions)if(C.Party.Contains(Partner.IndividualId))Partner.Memories.AddUnique(TEXT("garden.first_rain"));
    }
    else if(Event==TEXT("report") && Before==2)C.QuestStages.Add(TEXT("main.garden"),3);
    else return false;
    if(!FVZSaveStore::Validate(C))return false;S=MoveTemp(C);return true;
}
FString FVZGardenRules::Objective(const FVZWorldState& S)
{
    switch(Stage(S))
    {
    case 0:return TEXT("前往无访花庭 · 北侧通道交互");
    case 1:return Aligned(S)?TEXT("光路已通 · 在花冠旁交互，让菌翼蛾打开防护"):TEXT("扫描三座导光装置，按线索校准光路");
    case 2:return TEXT("花庭恢复供水 · 回访入口档案台");
    default:return TEXT("花庭原型流程完成 · 可回访温室");
    }
}
TArray<TPair<FVector,FString>> FVZGardenRules::Targets(const FVZWorldState& S)
{
    TArray<TPair<FVector,FString>> T;
    if(Stage(S)==0)T.Emplace(Entrance,TEXT("无访花庭"));
    else if(Stage(S)==1)
    {for(int32 I=0;I<3;++I)if(S.QuestStages.FindRef(Key(I))!=Correct[I])T.Emplace(Mirrors[I],FString::Printf(TEXT("导光装置 %d"),I+1));if(T.IsEmpty())T.Emplace(Flower,TEXT("传粉花冠"));}
    else T.Emplace(Archive,TEXT("花庭档案"));return T;
}
AVZGardenGuardian::AVZGardenGuardian()
{
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Leaf(TEXT("/Game/VariantZero/Guardians/SM_GuardianLeaf"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Jade(TEXT("/Game/VariantZero/Environment/Materials/M_LivingLeaf"));
    Core->SetRelativeScale3D(FVector(1));
    for(int32 I=0;I<6;++I)
    {
        if(auto* Rib=Cast<USceneComponent>(GetDefaultSubobjectByName(*FString::Printf(TEXT("ContainmentRib%d"),I))))Rib->SetVisibility(false);
        auto* LeafPart=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("GuardianPetal%d"),I));
        LeafPart->SetupAttachment(Core);LeafPart->SetStaticMesh(Leaf.Object);LeafPart->SetMaterial(0,Jade.Object);
        LeafPart->SetRelativeScale3D(FVector(.95f,.28f,2.5f));LeafPart->SetCollisionEnabled(ECollisionEnabled::NoCollision);LeafPart->SetCanEverAffectNavigation(false);
        Petals.Add(LeafPart);
    }
    UpdatePetals(0);
}
void AVZGardenGuardian::UpdatePetals(float Dt)
{
    const float Target=(Cleared || ExposureSeconds>0)?1.f:0.f;
    PetalOpen=FMath::FInterpTo(PetalOpen,Target,Dt,5.f);
    for(int32 I=0;I<Petals.Num();++I)
    {
        const float Angle=I*PI/3;
        Petals[I]->SetRelativeLocation(FVector(FMath::Cos(Angle)*(75+70*PetalOpen),FMath::Sin(Angle)*(75+70*PetalOpen),25-25*PetalOpen));
        Petals[I]->SetRelativeRotation(FRotator(0,I*60-90,15-90*PetalOpen));
    }
}
void AVZGardenGuardian::Start(AVZEcologist* InPlayer)
{
    if(Active || !InPlayer)return;
    ResetEncounter();Super::Start(InPlayer);
}
void AVZGardenGuardian::ResetEncounter()
{
    Super::ResetEncounter();Phase=1;ExposureSeconds=0;WarningCenters.Reset();WarningRadius=165;
}
bool AVZGardenGuardian::OpenExposure()
{
    if(!Active || ExposureSeconds>0)return false;
    ExposureSeconds=8;return true;
}
bool AVZGardenGuardian::Hit(float Damage,AActor* Source)
{
    if(!Active || !IsValid(Source) || !FMath::IsFinite(Damage) || Damage<=0)return false;
    if(ExposureSeconds<=0){if(Player)Player->State()->LastMessage=TEXT("防护仍在：带菌翼蛾回花冠旁打开它。");return false;}
    // The midpoint is a mandatory ecology phase; a large hit cannot skip it.
    const float Allowed=Phase==1?FMath::Min(Damage,FMath::Max(0.f,Vital->Health-90)):Damage;
    const bool Applied=Super::Hit(Allowed,Source);
    if(Applied && Phase==1 && Vital->Health<=90)
    {
        Phase=2;ExposureSeconds=0;WindingUp=false;WarningCenters.Reset();AttackClock=1.5f;
        if(Player)Player->State()->LastMessage=TEXT("守护者重新合拢叶冠。再次传粉破防；三处根刺会同时落下，离开橙色预警区。");
    }
    if(Cleared){ExposureSeconds=0;WarningCenters.Reset();}
    return Applied;
}
void AVZGardenGuardian::Tick(float Dt)
{
    // Own attack schedule; the training node's single-circle attack is not also run.
    AActor::Tick(Dt);
    ExposureSeconds=FMath::Max(0.f,ExposureSeconds-Dt);UpdatePetals(Dt);
    if(!Active || !IsValid(Player))return;
    if(Player->Vital->IsDown() || FVector::Dist2D(Player->GetActorLocation(),GetActorLocation())>1800){ResetEncounter();return;}
    AttackClock-=Dt;
    if(!WindingUp && AttackClock<=0)
    {
        WindingUp=true;AttackClock=Phase==1?1.1f:1.4f;ImpactPoint=Player->GetActorLocation();ImpactPoint.Z=8;
        WarningCenters={ImpactPoint};WarningRadius=Phase==1?165.f:125.f;
        if(Phase==2){WarningCenters.Add(ImpactPoint+FVector(280,0,0));WarningCenters.Add(ImpactPoint-FVector(280,0,0));}
    }
    if(!WindingUp)return;
    for(const auto& Center:WarningCenters)DrawDebugCircle(GetWorld(),Center,WarningRadius,48,FColor(255,120,45),false,0,0,3,FVector(1,0,0),FVector(0,1,0),false);
    if(AttackClock>0)return;
    const float Damage=Player->State()->State.StoryDifficulty?18.f:30.f;
    const auto InWarning=[this](const FVector& Position)
    {
        if(Position.Z>=200)return false;
        for(const auto& Center:WarningCenters)if(FVector::Dist2D(Position,Center)<WarningRadius)return true;
        return false;
    };
    if(InWarning(Player->GetActorLocation()))Player->Vital->ReceiveDamage(Damage);
    for(auto C:Player->Companions)if(IsValid(C) && InWarning(C->GetActorLocation()))C->Vital->ReceiveDamage(Damage);
    for(const auto& Center:WarningCenters)DrawPulse(GetWorld(),GetActorLocation(),Center+FVector(0,0,35),true);
    WindingUp=false;WarningCenters.Reset();AttackClock=Player->State()->State.StoryDifficulty?3.f:2.2f;
}
UVZGarden::UVZGarden(){PrimaryComponentTick.bCanEverTick=true;}
AVZEcologist* UVZGarden::Player() const{return Cast<AVZEcologist>(GetOwner());}
bool UVZGarden::Enabled() const{return GetWorld() && GetWorld()->GetMapName().Contains(TEXT("_Garden")) && Player() && Player()->Prologue->Enabled();}
bool UVZGarden::Apply(const FString& Event)
{
    auto C=Player()->State()->State;if(!FVZGardenRules::Apply(C,Event) || !Player()->State()->Commit(C))return false;
    Refresh();return true;
}
void UVZGarden::Refresh()
{
    if(!Enabled())return;
    if(!Guardian)
    {Training=Player()->Disturbance;Guardian=GetWorld()->SpawnActor<AVZGardenGuardian>(FVector(0,5600,130),FRotator::ZeroRotator);}
    for(auto A:Markers)if(IsValid(A))A->Destroy();Markers.Reset();
    const auto& S=Player()->State()->State;
    if(FVZGardenRules::Stage(S)>=2)
    {
        Guardian->ResetEncounter();Guardian->Cleared=true;Guardian->Vital->Health=0;
    }
    for(int32 I=0;I<3;++I)
    {
        auto* A=GetWorld()->SpawnActor<AStaticMeshActor>(Mirrors[I],FRotator::ZeroRotator);Markers.Add(A);
        auto* M=A->GetStaticMeshComponent();M->SetMobility(EComponentMobility::Movable);M->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cube")));M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetCanEverAffectNavigation(false);
        M->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/VariantZero/Environment/Materials/M_Biolight")));
        A->SetActorScale3D(FVector(1.5f,.16f,1.4f));A->SetActorRotation(FRotator(0,S.QuestStages.FindRef(Key(I))*90,0));
    }
    for(TActorIterator<AStaticMeshActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("VZGardenShortcut")))
    {It->SetActorHiddenInGame(S.Repairs.Contains(TEXT("garden.shortcut")));It->SetActorEnableCollision(!S.Repairs.Contains(TEXT("garden.shortcut")));}
    for(TActorIterator<AStaticMeshActor> It(GetWorld());It;++It)if(It->ActorHasTag(TEXT("VZGardenWater")))
        It->SetActorHiddenInGame(!S.Repairs.Contains(TEXT("garden.waterway")));
    Player()->ChuyaStory->Refresh();
}
void UVZGarden::StartEncounter()
{
    if(!Enabled() || !Guardian || Player()->Prologue->DialogueOpen() || FVZGardenRules::Stage(Player()->State()->State)!=1)return;
    Player()->Disturbance=Guardian;
    if(!Guardian->Active && !Guardian->Cleared){Guardian->ExposureSeconds=0;Guardian->Start(Player());}
}
bool UVZGarden::Scan()
{
    if(!Enabled() || Player()->GetActorLocation().Y<1800)return false;
    for(int32 I=0;I<3;++I)if(FVector::Dist2D(Player()->GetActorLocation(),Mirrors[I])<260)
    {
        const TCHAR* Directions[]={TEXT("北"),TEXT("东"),TEXT("南"),TEXT("西")};
        Player()->State()->LastMessage=FString::Printf(TEXT("导光装置 %d：旧日志要求朝%s；目前朝%s。交互每次顺时针转动一格。"),I+1,Directions[Correct[I]],Directions[Player()->State()->State.QuestStages.FindRef(Key(I))%4]);return true;
    }
    return false;
}
bool UVZGarden::Interact()
{
    if(!Enabled() || Player()->Prologue->DialogueOpen())return false;
    auto* P=Player();const auto Pos=P->GetActorLocation();const int32 Stage=FVZGardenRules::Stage(P->State()->State);
    if(Stage==0 && P->State()->State.Repairs.Contains(TEXT("prologue.pollinated")) && FVector::Dist2D(Pos,Entrance)<240)
    {
        if(Apply(TEXT("enter"))){P->SetActorLocation(P->State()->State.Checkpoint);P->RebuildParty();P->Prologue->Show(TEXT("无访花庭 · 锁住的阳光"),TEXT("园丁记录：守护者把所有花房都留在同一个正午。\n\n先扫描三座导光装置，恢复各自需要的光；再带菌翼蛾靠近花冠。防护打开后，只有短暂的时间让伙伴协同压制。"));}
        else P->State()->LastMessage=TEXT("先完成温室传粉，并带上菌翼蛾。");return true;
    }
    if(Pos.Y<1800)return false;
    if(Stage==0){P->State()->LastMessage=TEXT("请先在北侧通道接受花庭委托。");return true;}
    if(FVector::Dist2D(Pos,Archive)<220)
    {
        if(Stage==2 && !Apply(TEXT("report")))return true;
        P->Prologue->Show(TEXT("花庭档案"),Stage>=2?TEXT("水再次流向不同的花。我们恢复的不是旧日的正午，而是生命回应今天的机会。\n\n已解锁侧面捷径；队员共同经历与研究样本已经保存。\n\n当前仍为关卡原型，后续地区尚未开放。"):TEXT("三座导光装置留下了各自的朝向记录。先扫描，再校准。花冠需要健康的菌翼蛾在附近；防护每次只打开八秒。"));return true;
    }
    if(Stage==1)
    {
        for(int32 I=0;I<3;++I)if(FVector::Dist2D(Pos,Mirrors[I])<220)
        {if(Apply(FString::Printf(TEXT("mirror.%d"),I))){Scan();if(Guardian->Active)Guardian->ExposureSeconds=0;}return true;}
        if(FVector::Dist2D(Pos,Flower)<250)
        {
            if(!FVZGardenRules::Aligned(P->State()->State)){P->State()->LastMessage=TEXT("光路尚未连通，花冠没有回应。");return true;}
            AVZCompanionProxy* Moth=nullptr;for(auto C:P->Companions)if(C->SpeciesId==TEXT("V-041") && !C->Vital->IsDown() && FVector::Dist2D(C->GetActorLocation(),Flower)<450)Moth=C;
            if(!Moth){P->State()->LastMessage=TEXT("等待健康的菌翼蛾来到花冠附近。");return true;}
            StartEncounter();if(Guardian->OpenExposure()){AVZDisturbance::DrawPulse(GetWorld(),Moth->GetActorLocation()+FVector(0,0,85),Flower,true);P->State()->LastMessage=TEXT("菌翼蛾打开了防护：八秒内协同攻击，注意地面预警。");}else if(Guardian->Active)P->State()->LastMessage=TEXT("防护已经打开，趁现在协同攻击；交互不会延长窗口。");return true;
        }
    }
    P->State()->LastMessage=FVZGardenRules::Objective(P->State()->State);return true;
}
void UVZGarden::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Fn)
{
    Super::TickComponent(Dt,Type,Fn);if(!Enabled() || !Guardian)return;
    auto* P=Player();P->Disturbance=P->GetActorLocation().Y>1800?Guardian.Get():Training.Get();
    if(Guardian->Cleared && FVZGardenRules::Stage(P->State()->State)==1)
    {if(Apply(TEXT("repair"))){AVZAudioDirector::PlayCue(GetWorld(),TEXT("Repair"));P->State()->LastMessage=TEXT("供水恢复，侧面捷径已打开。获得 4 份研究样本；回到入口档案台报告。");}}
}
