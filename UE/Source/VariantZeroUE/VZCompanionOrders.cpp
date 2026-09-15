#include "VZPrototype.h"
#include "VZCombat.h"
#include "VZState.h"
#include "VZRoster.h"
#include "VZPrologue.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "DrawDebugHelpers.h"
#include "Animation/AnimSequence.h"
#include "Components/SkeletalMeshComponent.h"

void AVZCompanionProxy::CancelOrder()
{
    HasGroundOrder=false;OrderSeconds=0;RepathSeconds=0;
    if(auto* AI=Cast<AAIController>(GetController()))AI->StopMovement();
    GetCharacterMovement()->StopMovementImmediately();
}
bool AVZCompanionProxy::TickGroundOrder(float Dt)
{
    RecallSeconds=FMath::Max(0.f,RecallSeconds-Dt);
    if(!HasGroundOrder)return false;
    auto* P=Cast<AVZEcologist>(Leader);
    if(!P||Vital->IsDown()){CancelOrder();return false;}
    OrderSeconds+=Dt;
    DrawDebugCircle(GetWorld(),GroundOrder+FVector(0,0,6),220,48,FColor(90,255,170),false,-1,0,3,FVector::ForwardVector,FVector::RightVector,false);
    const bool Moving=GetVelocity().Size2D()>12;
    if(Moving!=Walking){Walking=Moving;if(IdleAnimation&&WalkAnimation)GetMesh()->PlayAnimation(Walking?WalkAnimation:IdleAnimation,true);}
    if(FVector::Dist2D(GetActorLocation(),GroundOrder)<90)
    {
        CancelOrder();++GroundSkillsCompleted;P->CompanionSkillCooldown=6;
        DrawDebugSphere(GetWorld(),GroundOrder+FVector(0,0,60),220,24,FColor(90,255,170),false,.6f,0,2);
        if(IsValid(P->Disturbance)&&P->Disturbance->Active&&FVector::Dist2D(GroundOrder,P->Disturbance->GetActorLocation())<=220&&AVZDisturbance::HasClearShot(this,P->Disturbance,GetActorLocation()+FVector(0,0,35)))
        {AVZDisturbance::DrawPulse(GetWorld(),GetActorLocation(),P->Disturbance->GetActorLocation(),true);P->Disturbance->Hit(FVZRosterRules::SkillDamage(FVZRosterRules::Level(P->State()->State,IndividualId)),this);}
        P->State()->LastMessage=TEXT("初芽已在指定位置释放技能，正在归队。");
        return true;
    }
    if(OrderSeconds>12||FVector::Dist2D(P->GetActorLocation(),GroundOrder)>1800)
    {CancelOrder();P->State()->LastMessage=TEXT("指令超时或距离过远，初芽归队；未消耗技能。");return false;}
    RepathSeconds-=Dt;
    if(RepathSeconds<=0)
    {if(auto* AI=Cast<AAIController>(GetController()))AI->MoveToLocation(GroundOrder,45,true,true,false,false,nullptr,false);RepathSeconds=.5f;}
    return true;
}
void AVZEcologist::BeginOrderAim(){Prologue->CancelPollination();AimingOrder=true;OrderAimValid=false;TickOrderAim();}
void AVZEcologist::TickOrderAim()
{
    if(!AimingOrder)return;
    OrderAimValid=false;auto* PC=Cast<APlayerController>(GetController());if(!PC)return;
    FVector Eye;FRotator View;PC->GetPlayerViewPoint(Eye,View);
    FCollisionQueryParams Query;Query.AddIgnoredActor(this);for(AVZCompanionProxy* C:Companions)Query.AddIgnoredActor(C);
    FHitResult Hit;
    if(GetWorld()->LineTraceSingleByChannel(Hit,Eye,Eye+View.Vector()*2500,ECC_Visibility,Query))
    {
        OrderAim=Hit.ImpactPoint;
        OrderAimValid=Hit.ImpactNormal.Z>.6f&&FVector::Dist2D(GetActorLocation(),OrderAim)<=1200;
        DrawDebugCircle(GetWorld(),OrderAim+FVector(0,0,8),220,48,OrderAimValid?FColor::Green:FColor::Red,false,-1,0,3,FVector::ForwardVector,FVector::RightVector,false);
    }
}
void AVZEcologist::ReleaseOrderAim()
{
    if(!AimingOrder)return;
    TickOrderAim();AimingOrder=false;
    if(OrderAimValid)IssueGroundOrder(OrderAim);
    else State()->LastMessage=TEXT("请指向 12 米内的地面，再松开指令键。");
}
bool AVZEcologist::IssueGroundOrder(FVector Point)
{
    if(Point.ContainsNaN()||Companions.IsEmpty()||Vital->IsDown())return false;
    if(CompanionSkillCooldown>0){State()->LastMessage=TEXT("伙伴技能正在冷却，请稍后下达指令。");return false;}
    auto* C=Companions[0].Get();if(!IsValid(C)||C->Vital->IsDown()){State()->LastMessage=TEXT("初芽已倒地，暂时无法执行指令。");return false;}
    auto Fail=[this](){State()->LastMessage=TEXT("指定位置不可达或距离过远；未消耗技能。");return false;};
    if(FVector::Dist2D(GetActorLocation(),Point)>1200)return Fail();
    auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());FNavLocation Projected;
    if(!Nav||!Nav->ProjectPointToNavigation(Point,Projected,FVector(50,50,90))){UE_LOG(LogTemp,Display,TEXT("VZ_ORDER_REJECT projection point=%s"),*Point.ToString());return Fail();}
    if(FVector::Dist2D(Point,Projected.Location)>65||FMath::Abs(Point.Z-Projected.Location.Z)>90){UE_LOG(LogTemp,Display,TEXT("VZ_ORDER_REJECT projection mismatch"));return Fail();}
    auto* Path=Nav->FindPathToLocationSynchronously(GetWorld(),C->GetActorLocation(),Projected.Location,C);
    if(!Path||!Path->IsValid()||Path->IsPartial()){UE_LOG(LogTemp,Display,TEXT("VZ_ORDER_REJECT path start=%s target=%s valid=%d partial=%d"),*C->GetActorLocation().ToString(),*Projected.Location.ToString(),Path&&Path->IsValid(),Path&&Path->IsPartial());return Fail();}
    C->CancelOrder();C->GroundOrder=Projected.Location;C->HasGroundOrder=true;C->RecallSeconds=0;
    State()->LastMessage=TEXT("初芽前往技能落点；召回动作可取消。");return true;
}
void AVZEcologist::RecallParty()
{
    Prologue->CancelPollination();
    AimingOrder=false;OrderAimValid=false;
    for(AVZCompanionProxy* C:Companions)if(IsValid(C)){C->CancelOrder();C->RecallSeconds=2;}
    State()->LastMessage=TEXT("伙伴已收到召回指令，暂时停止攻击并归队。");
}
