#pragma once
#include "CoreMinimal.h"
#include "VZState.h"

class VARIANTZEROUE_API FVZTravelRules
{
public:
    static bool Destination(const FVZWorldState& State,const FString& Id,FVector& Position);
    static bool Apply(FVZWorldState& State,const FString& Id);
    static FString Record(const FVZWorldState& State,int32 Index);
};
