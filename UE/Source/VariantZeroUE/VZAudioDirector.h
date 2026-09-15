#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "VZAudioDirector.generated.h"
class UAudioComponent;
class USoundWave;
UCLASS()
class VARIANTZEROUE_API AVZAudioDirector : public AActor
{
    GENERATED_BODY()
public:
    AVZAudioDirector();
    virtual void BeginPlay() override;
    virtual void Tick(float Dt) override;
    UPROPERTY() TObjectPtr<UAudioComponent> Calm;
    UPROPERTY() TObjectPtr<UAudioComponent> Alert;
    UPROPERTY() TObjectPtr<UAudioComponent> Exploration;
    UPROPERTY() TObjectPtr<USoundWave> Pulse;
    UPROPERTY() TObjectPtr<USoundWave> Shield;
    UPROPERTY() TObjectPtr<USoundWave> Repair;
    UPROPERTY() TObjectPtr<USoundWave> Hit;
    float Mix=0;
    float RegionMix=0;
    double LastHitTime=-1;
    void ApplyMix();
    static void PlayCue(UWorld* World,FName Name);
};
