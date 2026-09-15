#include "VZCombat.h"
#include "VZAudioDirector.h"
#include "VZHeroAnimation.h"
#include "Components/SkeletalMeshComponent.h"
#include "VZPrototype.h"
#include "VZState.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "Materials/MaterialInterface.h"
#include "DrawDebugHelpers.h"
#include "UObject/ConstructorHelpers.h"

UVZVitalComponent::UVZVitalComponent() { PrimaryComponentTick.bCanEverTick=true; }
void UVZVitalComponent::TickComponent(float Dt,ELevelTick Type,FActorComponentTickFunction* Function)
{
    Super::TickComponent(Dt,Type,Function);
    ShieldSeconds=FMath::Max(0.f,ShieldSeconds-Dt); InvulnerableSeconds=FMath::Max(0.f,InvulnerableSeconds-Dt);
}
float UVZVitalComponent::ReceiveDamage(float Amount)
{
    if (!FMath::IsFinite(Amount) || Amount<=0 || IsDown() || InvulnerableSeconds>0) return 0;
    const float Applied=FMath::Min(Health,Amount*(ShieldSeconds>0?.2f:1.f)); Health-=Applied;
    if(Applied>0)AVZAudioDirector::PlayCue(GetWorld(),TEXT("Hit"));
    if(Applied>0)if(auto* Hero=Cast<AVZEcologist>(GetOwner()))
        if(auto* Anim=Cast<UVZHeroAnimation>(Hero->GetMesh()->GetAnimInstance()))Anim->RequestAction(2);
    return Applied;
}
void UVZVitalComponent::Restore() { Health=MaxHealth;ShieldSeconds=0;InvulnerableSeconds=0; }

AVZDisturbance::AVZDisturbance()
{
    PrimaryActorTick.bCanEverTick=true;
    Core=CreateDefaultSubobject<UStaticMeshComponent>(TEXT("UnstableCore"));SetRootComponent(Core);
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Sphere(TEXT("/Engine/BasicShapes/Sphere"));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> Cylinder(TEXT("/Engine/BasicShapes/Cylinder"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Jade(TEXT("/Game/VariantZero/Environment/Materials/M_LivingLeaf"));
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> Glow(TEXT("/Game/VariantZero/Environment/Materials/M_Biolight"));
    Core->SetStaticMesh(Sphere.Object);Core->SetMaterial(0,Glow.Object);Core->SetRelativeScale3D(FVector(.9,.9,1.25));
    for (int32 I=0;I<6;++I)
    {
        auto* Rib=CreateDefaultSubobject<UStaticMeshComponent>(*FString::Printf(TEXT("ContainmentRib%d"),I));Rib->SetupAttachment(Core);
        Rib->SetStaticMesh(Cylinder.Object);Rib->SetMaterial(0,Jade.Object);
        const float Angle=I*PI/3;
        Rib->SetRelativeLocation(FVector(FMath::Cos(Angle)*70,FMath::Sin(Angle)*70,-25));
        Rib->SetRelativeScale3D(FVector(.1,.1,1.7));Rib->SetRelativeRotation(FRotator(20,I*60,0));
        Rib->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    Vital=CreateDefaultSubobject<UVZVitalComponent>(TEXT("Vital"));Vital->MaxHealth=180;Vital->Health=180;
}
void AVZDisturbance::Start(AVZEcologist* InPlayer)
{
    if (Active || !InPlayer) return;
    Player=InPlayer;Vital->Restore();Active=true;Cleared=false;WindingUp=false;Marked=false;AttackClock=2;
    Player->State()->LastMessage=TEXT("异常开始响应：标记后发射脉冲，使用伙伴技能与护盾。");
}
void AVZDisturbance::ResetEncounter() { Active=false;Cleared=false;WindingUp=false;Marked=false;Vital->Restore(); }
bool AVZDisturbance::HasClearShot(AActor* From,AActor* Target,FVector Origin)
{
    if (!IsValid(From) || !IsValid(Target)) return false;
    FHitResult Hit;FCollisionQueryParams Params;Params.AddIgnoredActor(From);
    if (auto* C=Cast<AVZCompanionProxy>(From)) Params.AddIgnoredActor(C->Leader);
    return !From->GetWorld()->LineTraceSingleByChannel(Hit,Origin,Target->GetActorLocation(),ECC_Visibility,Params) || Hit.GetActor()==Target;
}
void AVZDisturbance::DrawPulse(UWorld* World,FVector Start,FVector End,bool Strong)
{
    auto* Beam=World->SpawnActor<AStaticMeshActor>((Start+End)*.5,(End-Start).Rotation());
    auto* Mesh=Beam->GetStaticMeshComponent();Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetStaticMesh(LoadObject<UStaticMesh>(nullptr,TEXT("/Engine/BasicShapes/Cylinder.Cylinder")));
    Mesh->SetMaterial(0,LoadObject<UMaterialInterface>(nullptr,TEXT("/Game/VariantZero/Environment/Materials/M_Biolight.M_Biolight")));
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    Beam->SetActorRotation(FRotationMatrix::MakeFromZ(End-Start).Rotator());
    Beam->SetActorScale3D(FVector(Strong?.11f:.035f,Strong?.11f:.035f,FVector::Distance(Start,End)/100));Beam->SetLifeSpan(Strong?.3f:.13f);
}
bool AVZDisturbance::Hit(float Damage,AActor* Source)
{
    if (!Active || !IsValid(Source)) return false;
    Vital->ReceiveDamage(Damage);
    if (Vital->IsDown())
    {
        Active=false;Cleared=true;WindingUp=false;
        if (Player)
        {
            for (AVZCompanionProxy* C:Player->Companions) if (IsValid(C)) C->Vital->Restore();
            Player->State()->LastMessage=TEXT("异常已平息。初芽恢复了精神；可以照料花床，或按 T 再练习。");
        }
    }
    return true;
}
void AVZDisturbance::Tick(float Dt)
{
    Super::Tick(Dt);
    if (!Active || !IsValid(Player)) return;
    if (FVector::Dist2D(Player->GetActorLocation(),GetActorLocation())>1800) { ResetEncounter();Player->State()->LastMessage=TEXT("已离开异常范围，节点回到初始状态。");return; }
    AttackClock-=Dt;
    if (!WindingUp && AttackClock<=0)
    {
        WindingUp=true;AttackClock=1.1f;ImpactPoint=Player->GetActorLocation();ImpactPoint.Z=8;
    }
    if (WindingUp)
    {
        DrawDebugCircle(GetWorld(),ImpactPoint,165,48,FColor(255,120,45),false,0,0,3,FVector(1,0,0),FVector(0,1,0),false);
        if (AttackClock<=0)
        {
            const float Damage=Player->State()->State.StoryDifficulty?18.f:30.f;
            if (FVector::Dist2D(Player->GetActorLocation(),ImpactPoint)<165 && Player->GetActorLocation().Z<200) Player->Vital->ReceiveDamage(Damage);
            for (AVZCompanionProxy* C:Player->Companions)
                if (IsValid(C) && FVector::Dist2D(C->GetActorLocation(),ImpactPoint)<165) C->Vital->ReceiveDamage(Damage);
            DrawPulse(GetWorld(),GetActorLocation(),ImpactPoint+FVector(0,0,35),true);
            WindingUp=false;AttackClock=2.2f;
        }
    }
}
