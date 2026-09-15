#pragma once
#include "CoreMinimal.h"
#include "VZState.h"
#include "Components/ActorComponent.h"
#include "VZChuyaStory.generated.h"
class AVZEcologist;

class VARIANTZEROUE_API FVZChuyaStoryRules
{
public:
    static int32 Stage(const FVZWorldState& S);
    static FGuid Protagonist(const FVZWorldState& S);
    static bool Apply(FVZWorldState& S,const FString& Event);
    static FString Journal(const FVZWorldState& S);
    static FString Objective(const FVZWorldState& S);
    static TArray<TPair<FVector,FString>> Targets(const FVZWorldState& S);
};

UCLASS()
class VARIANTZEROUE_API UVZChuyaStory : public UActorComponent
{
    GENERATED_BODY()
public:
    bool Enabled() const;
    bool Interact();
    bool Scan();
    void Refresh();
private:
    AVZEcologist* Player() const;
    bool ReadyPartner() const;
    bool Apply(const FString& Event);
    UPROPERTY() TArray<TObjectPtr<AActor>> Props;
};
