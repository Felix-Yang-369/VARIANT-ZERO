#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Actor.h"
#include "VZCombat.generated.h"
class AVZEcologist;
class UStaticMeshComponent;

UCLASS(ClassGroup=(VariantZero),meta=(BlueprintSpawnableComponent))
class VARIANTZEROUE_API UVZVitalComponent : public UActorComponent
{
    GENERATED_BODY()
public:
    UVZVitalComponent();
    virtual void TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Function) override;
    UPROPERTY(BlueprintReadOnly) float Health=100;
    UPROPERTY(BlueprintReadOnly) float MaxHealth=100;
    UPROPERTY(BlueprintReadOnly) float ShieldSeconds=0;
    UPROPERTY(BlueprintReadOnly) float InvulnerableSeconds=0;
    UFUNCTION(BlueprintCallable) float ReceiveDamage(float Amount);
    UFUNCTION(BlueprintCallable) void Restore();
    bool IsDown() const { return Health<=0; }
};

// A tool-validation encounter, not a finished guardian or the 20-round drill minigame.
UCLASS()
class VARIANTZEROUE_API AVZDisturbance : public AActor
{
    GENERATED_BODY()
public:
    AVZDisturbance();
    virtual void Tick(float Dt) override;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UStaticMeshComponent> Core;
    UPROPERTY(VisibleAnywhere) TObjectPtr<UVZVitalComponent> Vital;
    UPROPERTY() TObjectPtr<AVZEcologist> Player;
    bool Active=false;
    bool Cleared=false;
    bool WindingUp=false;
    bool Marked=false;
    float AttackClock=2;
    FVector ImpactPoint;
    virtual void Start(AVZEcologist* InPlayer);
    virtual void ResetEncounter();
    virtual bool Hit(float Damage,AActor* Source);
    static bool HasClearShot(AActor* From,AActor* Target,FVector Origin);
    static void DrawPulse(UWorld* World,FVector Start,FVector End,bool Strong=false);
};
