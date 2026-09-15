#include "VZHeroAnimation.h"
#include "Animation/AnimInstanceProxy.h"
#include "Animation/AnimNode_SequencePlayer.h"
#include "Animation/AnimSequence.h"
#include "Animation/AnimationPoseData.h"
#include "AnimationRuntime.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/ConstructorHelpers.h"

struct FVZHeroProxy final : FAnimInstanceProxy
{
    FAnimNode_SequencePlayer_Standalone Players[5];
    FAnimNode_SequencePlayer_Standalone Actions[3];
    int32 ActiveAction=-1, SeenSerial=0;
    float ActionTime=0, ActionDuration=0, ActionWeight=0;
    int32 Current=0, Previous=0;
    float Alpha=1;
    FVZHeroProxy(UAnimInstance* Instance) : FAnimInstanceProxy(Instance) {}
    virtual void Initialize(UAnimInstance* Instance) override
    {
        FAnimInstanceProxy::Initialize(Instance);
        auto* Hero=CastChecked<UVZHeroAnimation>(Instance);
        for(int32 I=0;I<5;++I)
        {
            Players[I].SetSequence(Hero->Clips[I]);
            Players[I].SetLoopAnimation(I<2 || I==3);
            Players[I].Initialize_AnyThread(FAnimationInitializeContext(this));
        }
        for(int32 I=0;I<3;++I)
        {
            Actions[I].SetSequence(Hero->ActionClips[I]);Actions[I].SetLoopAnimation(false);
            Actions[I].SetPlayRate(I==0?1.25f:I==2?1.4f:1.f);
            Actions[I].Initialize_AnyThread(FAnimationInitializeContext(this));
        }
    }
    virtual void CacheBones() override
    {
        for(auto& Player:Players) Player.CacheBones_AnyThread(FAnimationCacheBonesContext(this));
        for(auto& Action:Actions) Action.CacheBones_AnyThread(FAnimationCacheBonesContext(this));
    }
    virtual void PreUpdate(UAnimInstance* Instance,float Dt) override
    {
        FAnimInstanceProxy::PreUpdate(Instance,Dt);
        auto* Hero=CastChecked<UVZHeroAnimation>(Instance);
        auto* Character=Cast<ACharacter>(Hero->TryGetPawnOwner());
        if(!Character) return;
        const FVector Velocity=Character->GetVelocity();
        Hero->GroundSpeed=Velocity.Size2D();
        const int32 Next=Character->GetCharacterMovement()->IsFalling() ? (Velocity.Z>120?2:(Velocity.Z < -120?4:3)) : (Hero->GroundSpeed>8?1:0);
        if(Next!=Current)
        {
            Previous=Current;Current=Next;Alpha=0;
            Players[Current].Initialize_AnyThread(FAnimationInitializeContext(this));
        }
        Alpha=FMath::Min(1.f,Alpha+Dt/.18f);
        Players[1].SetPlayRate(FMath::Clamp(Hero->GroundSpeed/420.f,.55f,1.8f));
        Hero->LocomotionState=Current;
        if(SeenSerial!=Hero->ActionSerial)
        {
            SeenSerial=Hero->ActionSerial;ActiveAction=Hero->RequestedAction;ActionTime=0;
            if(ActiveAction>=0)
            {
                Actions[ActiveAction].Initialize_AnyThread(FAnimationInitializeContext(this));
                ActionDuration=Hero->ActionClips[ActiveAction]->GetPlayLength()/(ActiveAction==0?1.25f:ActiveAction==2?1.4f:1.f);
            }
        }
        ActionTime+=Dt;
        if(ActiveAction>=0 && ActionTime>=ActionDuration)ActiveAction=-1;
        ActionWeight=ActiveAction<0?0:FMath::Min(FMath::Clamp(ActionTime/.08f,0.f,1.f),FMath::Clamp((ActionDuration-ActionTime)/.12f,0.f,1.f));
        Hero->ActiveAction=ActiveAction;Hero->ActionWeight=ActionWeight;
    }
    virtual void UpdateAnimationNode(const FAnimationUpdateContext& Context) override
    {
        Players[Current].Update_AnyThread(Context.FractionalWeight(Alpha));
        if(Alpha<1 && Previous!=Current) Players[Previous].Update_AnyThread(Context.FractionalWeight(1-Alpha));
        if(ActiveAction>=0)Actions[ActiveAction].Update_AnyThread(Context.FractionalWeight(ActionWeight));
    }
    virtual bool Evaluate(FPoseContext& Output) override
    {
        Players[Current].Evaluate_AnyThread(Output);
        if(Alpha<1 && Previous!=Current)
        {
            FPoseContext Old(this),New(this);
            Players[Previous].Evaluate_AnyThread(Old);
            Players[Current].Evaluate_AnyThread(New);
            FAnimationPoseData A(Old),B(New),Result(Output);
            FAnimationRuntime::BlendTwoPosesTogether(A,B,1-Alpha,Result);
        }
        if(ActiveAction>=0 && ActionWeight>0)
        {
            FPoseContext Layer(this);Actions[ActiveAction].Evaluate_AnyThread(Layer);
            if(ActiveAction==2)
            {
                // HitReact_Front is local-space additive, not an absolute pose.
                FAnimationPoseData Base(Output),Additive(Layer);
                FAnimationRuntime::AccumulateAdditivePose(Base,Additive,ActionWeight*.7f,AAT_LocalSpaceBase);
                Output.Pose.NormalizeRotations();
            }
            else
            {
                // Preserve the root, pelvis and legs, including while jumping or dodging.
                const auto& Bones=Output.Pose.GetBoneContainer();
                TArray<float> Weights;Weights.Init(0.f,Output.Pose.GetNumBones());
                for(FCompactPoseBoneIndex Bone:Output.Pose.ForEachBoneIndex())
                {
                    auto Parent=Bone;
                    while(Parent.GetInt()!=INDEX_NONE)
                    {
                        if(Bones.GetReferenceSkeleton().GetBoneName(Bones.MakeMeshPoseIndex(Parent).GetInt())==TEXT("spine_01"))
                        {Weights[Bone.GetInt()]=ActionWeight;break;}
                        Parent=Bones.GetParentBoneIndex(Parent);
                    }
                }
                FPoseContext Blended(this);
                FAnimationPoseData Base(Output),Action(Layer),Result(Blended);
                FAnimationRuntime::BlendTwoPosesTogetherPerBone(Base,Action,Weights,Result);
                Output=MoveTemp(Blended);
            }
        }
        return true;
    }
};

UVZHeroAnimation::UVZHeroAnimation()
{
    const TCHAR* Names[]={TEXT("Idle_Relaxed"),TEXT("Jog_Fwd"),TEXT("Jump_Start"),TEXT("Jump_Apex"),TEXT("Jump_PreLand")};
    for(const TCHAR* Name:Names)
    {
        ConstructorHelpers::FObjectFinder<UAnimSequence> Clip(*(FString(TEXT("/Game/ParagonLtBelica/Characters/Heroes/Belica/Animations/"))+Name));
        Clips.Add(Clip.Object);
    }
    for(const TCHAR* Name:{TEXT("Primary_Fire_Fast"),TEXT("Cast"),TEXT("HitReact_Front")})
    {
        ConstructorHelpers::FObjectFinder<UAnimSequence> Clip(*(FString(TEXT("/Game/ParagonLtBelica/Characters/Heroes/Belica/Animations/"))+Name));
        ActionClips.Add(Clip.Object);
    }
}
void UVZHeroAnimation::RequestAction(int32 Action)
{
    if(!ActionClips.IsValidIndex(Action)||!ActionClips[Action])return;
    if(ActionWeight>.1f && (ActiveAction==Action || (ActiveAction>Action)))return;
    RequestedAction=Action;++ActionSerial;
}
void UVZHeroAnimation::ResetActions(){RequestedAction=-1;ActiveAction=-1;ActionWeight=0;++ActionSerial;}
FAnimInstanceProxy* UVZHeroAnimation::CreateAnimInstanceProxy(){return new FVZHeroProxy(this);}
void UVZHeroAnimation::DestroyAnimInstanceProxy(FAnimInstanceProxy* Proxy){delete Proxy;}
