#include "VZChuyaStory.h"
#include "VZPrototype.h"
#include "VZGarden.h"
#include "VZPrologue.h"
#include "VZAudioDirector.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "EngineUtils.h"

namespace
{
const FVector Archive(0,2500,90),Shallow(-450,3000,90),Dry(450,3000,90);
const FVector Sites[]={FVector(-1000,4800,90),FVector(-1100,6100,90),FVector(1050,5900,90)};
const TCHAR* Names[]={TEXT("低处的根"),TEXT("旧水位线"),TEXT("高处的新叶")};
const TCHAR* Notes[]={TEXT("初芽停在积水边，没有继续向前。根部需要水，也需要空气。连续浸泡会让新根无法呼吸。"),TEXT("旧水位线之上还有活着的苔藓。过去这里曾有涨落；永远相同的水位让变化消失了。"),TEXT("高处的嫩叶正在展开。这里排水快，却容易在日晒后干裂。水多一点或少一点，都不是所有生命的答案。")};
FString Observation(int32 I){return FString::Printf(TEXT("story.chuya.observation.%d"),I);}
}
int32 FVZChuyaStoryRules::Stage(const FVZWorldState& S){return S.QuestStages.FindRef(TEXT("story.chuya"));}
FGuid FVZChuyaStoryRules::Protagonist(const FVZWorldState& S)
{
    for(const auto& C:S.Companions)if(C.Memories.Contains(TEXT("story.chuya.protagonist")))return C.IndividualId;
    for(const auto& C:S.Companions)if(C.Memories.Contains(TEXT("origin.first_companion")))return C.IndividualId;
    // Older prototype saves retained companions in acquisition order.
    for(const auto& C:S.Companions)if(C.SpeciesId==TEXT("V-001"))return C.IndividualId;
    return FGuid();
}
bool FVZChuyaStoryRules::Apply(FVZWorldState& S,const FString& Event)
{
    if(!FVZSaveStore::Validate(S))return false;
    auto C=S;const int32 Before=Stage(C);const auto Id=Protagonist(C);
    if(!Id.IsValid() || !C.Party.Contains(Id))return false;
    if(Event==TEXT("begin") && Before==0 && FVZGardenRules::Stage(C)>=3)
    {
        C.QuestStages.Add(TEXT("story.chuya"),1);
        for(auto& Partner:C.Companions)if(Partner.IndividualId==Id)Partner.Memories.AddUnique(TEXT("story.chuya.protagonist"));
    }
    else if(Before==1 && Event.StartsWith(TEXT("observe.")))
    {
        int32 Index=INDEX_NONE;for(int32 I=0;I<3;++I)if(Event==FString::Printf(TEXT("observe.%d"),I))Index=I;
        if(Index==INDEX_NONE || C.Repairs.Contains(Observation(Index)))return false;
        C.Repairs.Add(Observation(Index));
        if(C.Repairs.Contains(Observation(0)) && C.Repairs.Contains(Observation(1)) && C.Repairs.Contains(Observation(2)))C.QuestStages.Add(TEXT("story.chuya"),2);
    }
    else if(Before==2 && (Event==TEXT("choose.shallow") || Event==TEXT("choose.dry")))
    {
        const FString Receipt=TEXT("story.chuya.reward.v1");if(C.RewardTransactions.Contains(Receipt))return false;
        const FString Choice=Event==TEXT("choose.shallow")?TEXT("story.chuya.shallow"):TEXT("story.chuya.dry");
        C.Repairs.Add(Choice);C.RewardTransactions.Add(Receipt);C.Materials.FindOrAdd(TEXT("research.sample"))++;
        C.QuestStages.Add(TEXT("story.chuya"),3);
        for(auto& Partner:C.Companions)if(Partner.IndividualId==Id){Partner.Memories.AddUnique(TEXT("story.chuya.first_rain"));Partner.Memories.AddUnique(Choice);}
    }
    else return false;
    if(!FVZSaveStore::Validate(C))return false;S=MoveTemp(C);return true;
}
FString FVZChuyaStoryRules::Journal(const FVZWorldState& S)
{
    if(Stage(S)==0)return FVZGardenRules::Stage(S)>=3?TEXT("【第一场雨】初芽在新水边停了下来。带它回花庭档案台，观察这场修复留下了什么。"):TEXT("【初芽】尚未发现新的共同经历。");
    FString Text=TEXT("【第一场雨】\n管理员：供水恢复之后，生命仍会有各自的需要。别急着给初芽一个答案，先陪它看一看。\n");
    for(int32 I=0;I<3;++I)if(S.Repairs.Contains(Observation(I)))Text+=TEXT("\n")+FString(Names[I])+TEXT("：")+Notes[I]+TEXT("\n");
    if(Stage(S)==2)Text+=TEXT("\n两种修复都能留下不同的水土条件：西侧标记扩建浅水带，并保留透气边缘；东侧标记保留干燥岛，让雨水沿岛边流过。走到一处标记交互提交选择。选择会改变现场陈设，奖励相同且只领取一次。");
    if(Stage(S)==3)
    {
        Text+=S.Repairs.Contains(TEXT("story.chuya.shallow"))?TEXT("\n你扩建了浅水带，让耐湿生命留下，也为新根留出空气。初芽沿着水边慢慢走了一圈。"):TEXT("\n你保留了干燥岛，水从岛边经过，幼根有了不被淹没的落脚处。初芽把一片叶子朝向了阳光。");
        Text+=TEXT("\n管理员：它未必需要我们把世界变成最适合它的样子。能与不同的生命一起长大，也是一种照顾。\n\n伙伴经历已保存。获得 1 份研究样本，可继续探索或回研究所。");
    }
    return Text;
}
FString FVZChuyaStoryRules::Objective(const FVZWorldState& S)
{
    if(Stage(S)==0)return TEXT("可选伙伴故事 · 带初芽回花庭档案台");
    if(Stage(S)==1){int32 Count=0;for(int32 I=0;I<3;++I)Count+=S.Repairs.Contains(Observation(I))?1:0;return FString::Printf(TEXT("第一场雨 · 带初芽扫描水土变化 %d / 3"),Count);}
    return Stage(S)==2?TEXT("第一场雨 · 选择浅水带或干燥岛，交互提交修复"):TEXT("第一场雨已记入共同经历 · 可自由回访");
}
TArray<TPair<FVector,FString>> FVZChuyaStoryRules::Targets(const FVZWorldState& S)
{
    TArray<TPair<FVector,FString>> T;if(Stage(S)==0)T.Emplace(Archive,TEXT("初芽 · 第一场雨"));
    if(Stage(S)==1)for(int32 I=0;I<3;++I)if(!S.Repairs.Contains(Observation(I)))T.Emplace(Sites[I],Names[I]);
    if(Stage(S)==2){T.Emplace(Shallow,TEXT("扩建浅水带"));T.Emplace(Dry,TEXT("保留干燥岛"));}return T;
}
AVZEcologist* UVZChuyaStory::Player() const{return Cast<AVZEcologist>(GetOwner());}
bool UVZChuyaStory::Enabled() const{return Player() && Player()->Garden->Enabled() && FVZGardenRules::Stage(Player()->State()->State)>=3;}
bool UVZChuyaStory::ReadyPartner() const
{
    auto* P=Player();if(P->Vital->IsDown())return false;
    for(TActorIterator<AVZDisturbance> It(GetWorld());It;++It)if(It->Active)return false;
    for(auto C:P->Companions)if(IsValid(C) && C->IndividualId==FVZChuyaStoryRules::Protagonist(P->State()->State) && !C->Vital->IsDown() && FVector::Dist2D(C->GetActorLocation(),P->GetActorLocation())<650)return true;
    return false;
}
bool UVZChuyaStory::Apply(const FString& Event)
{
    auto C=Player()->State()->State;if(!FVZChuyaStoryRules::Apply(C,Event) || !Player()->State()->Commit(C))return false;
    Refresh();return true;
}
bool UVZChuyaStory::Interact()
{
    if(!Enabled() || Player()->Prologue->DialogueOpen())return false;
    auto* P=Player();const auto Pos=P->GetActorLocation();const int32 Stage=FVZChuyaStoryRules::Stage(P->State()->State);
    if(FVector::Dist2D(Pos,Archive)<220)
    {
        if(Stage==0){if(!ReadyPartner()){P->State()->LastMessage=TEXT("带健康的初芽靠近档案台，战斗结束后再一起出发。");return true;}if(!Apply(TEXT("begin")))return true;}
        P->Prologue->Show(TEXT("初芽 · 第一场雨"),FVZChuyaStoryRules::Journal(P->State()->State));return true;
    }
    if(Stage==2 && (FVector::Dist2D(Pos,Shallow)<180 || FVector::Dist2D(Pos,Dry)<180))
    {
        if(!ReadyPartner()){P->State()->LastMessage=TEXT("等待健康的初芽来到身旁，再一起完成修复。");return true;}
        if(Apply(FVector::Dist2D(Pos,Shallow)<180?TEXT("choose.shallow"):TEXT("choose.dry")))
        {AVZAudioDirector::PlayCue(GetWorld(),TEXT("Repair"));P->Prologue->Show(TEXT("第一场雨 · 不同的落脚处"),FVZChuyaStoryRules::Journal(P->State()->State));}return true;
    }
    return false;
}
bool UVZChuyaStory::Scan()
{
    if(!Enabled() || Player()->Prologue->DialogueOpen() || FVZChuyaStoryRules::Stage(Player()->State()->State)!=1)return false;
    for(int32 I=0;I<3;++I)if(FVector::Dist2D(Player()->GetActorLocation(),Sites[I])<220)
    {
        if(!ReadyPartner()){Player()->State()->LastMessage=TEXT("让健康的初芽靠近，一起观察这处变化。");return true;}
        if(Player()->State()->State.Repairs.Contains(Observation(I)))Player()->Prologue->Show(Names[I],Notes[I]);
        else if(Apply(FString::Printf(TEXT("observe.%d"),I)))Player()->Prologue->Show(Names[I],FVZChuyaStoryRules::Journal(Player()->State()->State));return true;
    }
    return false;
}
void UVZChuyaStory::Refresh()
{
    for(auto A:Props)if(IsValid(A))A->Destroy();Props.Reset();if(!Enabled())return;
    const auto& S=Player()->State()->State;const int32 Stage=FVZChuyaStoryRules::Stage(S);
    auto Prop=[this](FVector Pos,FVector Scale,const TCHAR* Shape,const TCHAR* Material,FName Tag)
    {
        auto* A=GetWorld()->SpawnActor<AStaticMeshActor>(Pos,FRotator::ZeroRotator);Props.Add(A);A->Tags.Add(Tag);
        auto* M=A->GetStaticMeshComponent();M->SetMobility(EComponentMobility::Movable);M->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,Shape));M->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,Material));M->SetCollisionEnabled(ECollisionEnabled::NoCollision);M->SetCanEverAffectNavigation(false);A->SetActorScale3D(Scale);
    };
    if(Stage==1)for(int32 I=0;I<3;++I)if(!S.Repairs.Contains(Observation(I)))Prop(Sites[I]-FVector(0,0,70),FVector(.45,.45,.4),TEXT("/Engine/BasicShapes/Cylinder"),TEXT("/Game/VariantZero/Environment/Materials/M_Brass"),TEXT("VZChuyaObservation"));
    if(Stage==2)for(const auto Pos:{Shallow,Dry})Prop(Pos-FVector(0,0,55),FVector(1,1,.7),TEXT("/Engine/BasicShapes/Cylinder"),TEXT("/Game/VariantZero/Environment/Materials/M_Biolight"),TEXT("VZChuyaChoice"));
    if(Stage==3)
    {
        const bool Wet=S.Repairs.Contains(TEXT("story.chuya.shallow"));const FName Tag=Wet?TEXT("VZChuyaShallow"):TEXT("VZChuyaDry");
        for(int32 I=0;I<3;++I)
        {
            FVector Pos(-400+I*400,3150,8);Prop(Pos,FVector(2.8,2.2,.12),TEXT("/Engine/BasicShapes/Cylinder"),Wet?TEXT("/Game/VariantZero/Environment/Materials/M_Biolight"):TEXT("/Game/VariantZero/Ecology/M_forest_ground_04"),Tag);
            Prop(Pos+FVector(Wet?130:0,0,10),FVector(.8,.8,.8),TEXT("/Game/VariantZero/Ecology/SM_fern_02"),TEXT("/Game/VariantZero/Ecology/M_fern_02"),Tag);
        }
    }
}
