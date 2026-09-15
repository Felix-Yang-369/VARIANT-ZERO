#pragma once
#include "CoreMinimal.h"
#include "VZState.h"
#include "Components/ActorComponent.h"
#include "Blueprint/UserWidget.h"
#include "VZPrologue.generated.h"

class UUserWidget;
class UTextBlock;
class AVZEcologist;
class AVZCompanionProxy;

UCLASS()
class VARIANTZEROUE_API UVZDialogueWidget : public UUserWidget
{
    GENERATED_BODY()
};

// Stable persisted IDs; this short opening prototype is not a completed P1 chapter.
class VARIANTZEROUE_API FVZPrologueRules
{
public:
    static int32 Stage(const FVZWorldState& S);
    static bool Apply(FVZWorldState& S, const FString& Event);
    static FString Objective(const FVZWorldState& S);
    static FString Journal(const FVZWorldState& S);
    static TArray<TPair<FVector,FString>> Targets(const FVZWorldState& S);
};

UCLASS()
class VARIANTZEROUE_API UVZPrologue : public UActorComponent
{
    GENERATED_BODY()
public:
    UVZPrologue();
    virtual void BeginPlay() override;
    virtual void TickComponent(float Dt, ELevelTick TickType, FActorComponentTickFunction* Fn) override;
    bool Enabled() const;
    bool Interact();
    bool Scan();
    bool Apply(const FString& Event);
    void Refresh();
    void CancelPollination();
    UFUNCTION() void Dismiss();
    void Show(const FString& Title, const FString& Text);
    FString Objective() const;
    bool DialogueOpen() const { return Dialogue != nullptr; }
private:
    AVZEcologist* Player() const;
    UPROPERTY() TObjectPtr<UUserWidget> Dialogue;
    UPROPERTY() TArray<TObjectPtr<AActor>> Markers;
    TWeakObjectPtr<AVZCompanionProxy> Pollinator;
    FVector PreviousFormation=FVector::ZeroVector;
    float PollinationSeconds=0;
};
