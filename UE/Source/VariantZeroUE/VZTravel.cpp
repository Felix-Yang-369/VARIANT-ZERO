#include "VZTravel.h"
#include "VZPrologue.h"
#include "VZGarden.h"
#include "VZChuyaStory.h"

bool FVZTravelRules::Destination(const FVZWorldState& S,const FString& Id,FVector& Position)
{
    if(!FVZSaveStore::Validate(S))return false;
    if(Id==TEXT("institute")){Position=FVector(0,0,120);return true;}
    if(Id==TEXT("garden") && FVZGardenRules::Stage(S)>=1){Position=FVector(0,2200,120);return true;}
    return false;
}
bool FVZTravelRules::Apply(FVZWorldState& S,const FString& Id)
{
    FVector Target;if(!Destination(S,Id,Target))return false;
    S.Checkpoint=Target;return true;
}
FString FVZTravelRules::Record(const FVZWorldState& S,int32 Index)
{
    if(Index==0)return FVZPrologueRules::Journal(S);
    if(Index==1)
    {
        switch(FVZGardenRules::Stage(S))
        {
        case 0:return TEXT("无访花庭尚未建立联络。先完成温室传粉，带菌翼蛾前往北侧通道。");
        case 1:return TEXT("园丁记录：被锁住的正午让花失去了回应变化的能力。\n当前任务：")+FVZGardenRules::Objective(S);
        case 2:return TEXT("供水已经恢复，中间捷径已打开。队伍获得共同经历与四份研究样本。\n尚未完成：回访入口档案台。");
        default:return TEXT("花庭档案已更新：恢复的不是旧日正午，而是生命回应今天的机会。供水、捷径与伙伴经历已保存。温室与花庭可以自由回访。");
        }
    }
    if(Index==2 || Index==3)
    {
        const FString Species=Index==2?TEXT("V-001"):TEXT("V-041");FString Text;
        for(const auto& C:S.Companions)if(C.SpeciesId==Species)
        {
            Text+=C.Name+TEXT("：");
            Text+=C.Memories.Contains(TEXT("garden.first_rain"))?TEXT("共同见证花庭重新供水。"):TEXT("等待一起创造新的经历。");
            Text+=TEXT("\n");
        }
        if(Index==2)Text+=TEXT("\n")+FVZChuyaStoryRules::Journal(S);
        return Text.IsEmpty()?TEXT("尚未结识这类伙伴。"):Text.TrimEnd();
    }
    return FString::Printf(TEXT("现有研究样本：%d\n花庭修复礼包：%s。回访与传送不会重复发放奖励。"),S.Materials.FindRef(TEXT("research.sample")),S.RewardTransactions.Contains(TEXT("garden.repair.v1"))?TEXT("已领取"):TEXT("尚未领取"));
}
