#include "VZRoster.h"
int32 FVZRosterRules::GrowthPoints(const FVZWorldState& S)
{
    const int32 Earned=(S.QuestStages.FindRef(TEXT("main.prologue"))>=6?2:0)
        +(S.QuestStages.FindRef(TEXT("main.garden"))>=2?3:0)
        +(S.QuestStages.FindRef(TEXT("main.garden"))>=3?3:0);
    return FMath::Max(0,Earned-S.QuestStages.FindRef(TEXT("growth.spent")));
}
int32 FVZRosterRules::Level(const FVZWorldState& S,FGuid Id)
{
    for(const auto& C:S.Companions)if(C.IndividualId==Id)return C.Level;
    return 1;
}
bool FVZRosterRules::Train(FVZWorldState& S,FGuid Id,int32 ExpectedLevel)
{
    if(!FVZSaveStore::Validate(S)||ExpectedLevel<1||ExpectedLevel>=20)return false;
    for(auto& C:S.Companions)if(C.IndividualId==Id)
    {
        if(C.Level!=ExpectedLevel||GrowthPoints(S)<C.Level)return false;
        S.QuestStages.FindOrAdd(TEXT("growth.spent"))+=C.Level;++C.Level;return true;
    }
    return false;
}
bool FVZRosterRules::SetDeployed(FVZWorldState& State,FGuid Id,bool Deploy)
{
    if(!FVZSaveStore::Validate(State)||!State.Companions.ContainsByPredicate([Id](const FVZCompanion& C){return C.IndividualId==Id;}))return false;
    const bool Present=State.Party.Contains(Id);
    if(Present==Deploy)return false;
    if(Deploy){if(State.Party.Num()>=3)return false;State.Party.Add(Id);}
    else {if(State.Party.Num()<=1)return false;State.Party.Remove(Id);}
    return true;
}
bool FVZRosterRules::Rename(FVZWorldState& State,FGuid Id,const FString& Input)
{
    const FString Name=Input.TrimStartAndEnd();
    if(!FVZSaveStore::Validate(State)||Name.IsEmpty()||Name.Len()>16)return false;
    for(TCHAR C:Name)if(C<32||C==127||C==0x2028||C==0x2029)return false;
    for(auto& C:State.Companions)if(C.IndividualId==Id){if(C.Name==Name)return false;C.Name=Name;return true;}
    return false;
}
bool FVZRosterRules::Move(FVZWorldState& State,FGuid Id,int32 Direction)
{
    if(!FVZSaveStore::Validate(State)||(Direction!=1&&Direction!=-1))return false;
    const int32 I=State.Party.IndexOfByKey(Id);
    if(I==INDEX_NONE||!State.Party.IsValidIndex(I+Direction))return false;
    State.Party.Swap(I,I+Direction);return true;
}
