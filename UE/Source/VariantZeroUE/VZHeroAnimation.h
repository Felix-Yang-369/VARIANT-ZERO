#pragma once
#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "VZHeroAnimation.generated.h"
class UAnimSequence;

// Project-owned locomotion: no dependency on the legacy Paragon character Blueprint.
UCLASS(Transient)
class VARIANTZEROUE_API UVZHeroAnimation : public UAnimInstance
{
    GENERATED_BODY()
public:
    UVZHeroAnimation();
    UPROPERTY() TArray<TObjectPtr<UAnimSequence>> Clips;
    UPROPERTY() TArray<TObjectPtr<UAnimSequence>> ActionClips;
    // 0 pulse, 1 shield, 2 hit. Read by the proxy on the game thread only.
    int32 RequestedAction=-1, ActionSerial=0, ActiveAction=-1;
    float ActionWeight=0;
    void RequestAction(int32 Action);
    void ResetActions();
    int32 LocomotionState = 0;
    float GroundSpeed = 0;
    virtual FAnimInstanceProxy* CreateAnimInstanceProxy() override;
    virtual void DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy) override;
};
