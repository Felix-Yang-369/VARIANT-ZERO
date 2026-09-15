#pragma once
#include "CoreMinimal.h"
#include "VZState.h"
#include "VZCombat.h"
#include "Components/ActorComponent.h"
#include "VZGarden.generated.h"
class AVZEcologist;

class VARIANTZEROUE_API FVZGardenRules
{
public:
    static int32 Stage(const FVZWorldState& S);
    static bool Aligned(const FVZWorldState& S);
    static bool Apply(FVZWorldState& S,const FString& Event);
    static FString Objective(const FVZWorldState& S);
    static TArray<TPair<FVector,FString>> Targets(const FVZWorldState& S);
};

UCLASS()
class VARIANTZEROUE_API AVZGardenGuardian : public AVZDisturbance
{
    GENERATED_BODY()
public:
    AVZGardenGuardian();
    float ExposureSeconds=0;
    int32 Phase=1;
    TArray<FVector> WarningCenters;
    float WarningRadius=165;
    bool OpenExposure();
    virtual void Start(AVZEcologist* InPlayer) override;
    virtual void ResetEncounter() override;
    virtual bool Hit(float Damage,AActor* Source) override;
    virtual void Tick(float Dt) override;
private:
    UPROPERTY() TArray<TObjectPtr<UStaticMeshComponent>> Petals;
    float PetalOpen=0;
    void UpdatePetals(float Dt);
};

UCLASS()
class VARIANTZEROUE_API UVZGarden : public UActorComponent
{
    GENERATED_BODY()
public:
    UVZGarden();
    bool Enabled() const;
    void Refresh();
    bool Interact();
    bool Scan();
    void StartEncounter();
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Fn) override;
    UPROPERTY() TObjectPtr<AVZGardenGuardian> Guardian;
    UPROPERTY() TObjectPtr<AVZDisturbance> Training;
private:
    AVZEcologist* Player() const;
    bool Apply(const FString& Event);
    UPROPERTY() TArray<TObjectPtr<AActor>> Markers;
};
