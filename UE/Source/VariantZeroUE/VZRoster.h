#pragma once
#include "VZState.h"
class VARIANTZEROUE_API FVZRosterRules
{
public:
    static bool Rename(FVZWorldState& State,FGuid Id,const FString& Name);
    static bool Move(FVZWorldState& State,FGuid Id,int32 Direction);
    static bool SetDeployed(FVZWorldState& State,FGuid Id,bool Deploy);
    static int32 GrowthPoints(const FVZWorldState& State);
    static int32 Level(const FVZWorldState& State,FGuid Id);
    static bool Train(FVZWorldState& State,FGuid Id,int32 ExpectedLevel);
    static float MaxHealth(int32 Level){return 100.f+5.f*(FMath::Clamp(Level,1,20)-1);}
    static float BasicDamage(int32 Level){return 5.f+.5f*(FMath::Clamp(Level,1,20)-1);}
    static float SkillDamage(int32 Level){return 32.f+2.f*(FMath::Clamp(Level,1,20)-1);}
    static float Healing(int32 Level){return 20.f+(FMath::Clamp(Level,1,20)-1);}
};
