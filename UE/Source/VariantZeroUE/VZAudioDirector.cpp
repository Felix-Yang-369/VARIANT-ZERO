#include "VZAudioDirector.h"
#include "VZPrototype.h"
#include "VZCombat.h"
#include "VZGarden.h"
#include "VZPlayerController.h"
#include "Components/AudioComponent.h"
#include "Sound/SoundWave.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
AVZAudioDirector::AVZAudioDirector()
{
    PrimaryActorTick.bCanEverTick=true;
    Calm=CreateDefaultSubobject<UAudioComponent>(TEXT("CalmScore"));SetRootComponent(Calm);Calm->bAutoActivate=false;
    Alert=CreateDefaultSubobject<UAudioComponent>(TEXT("AlertScore"));Alert->SetupAttachment(Calm);Alert->bAutoActivate=false;
    Exploration=CreateDefaultSubobject<UAudioComponent>(TEXT("BeyondRootsScore"));Exploration->SetupAttachment(Calm);Exploration->bAutoActivate=false;
    static ConstructorHelpers::FObjectFinder<USoundWave> Roots(TEXT("/Game/VariantZero/Audio/SW_BeyondRoots"));
    Exploration->SetSound(Roots.Object);Exploration->bAllowSpatialization=false;
    static ConstructorHelpers::FObjectFinder<USoundWave> A(TEXT("/Game/VariantZero/Audio/SW_Conservatory")),B(TEXT("/Game/VariantZero/Audio/SW_Alert")),C(TEXT("/Game/VariantZero/Audio/SW_Pulse")),D(TEXT("/Game/VariantZero/Audio/SW_Shield")),E(TEXT("/Game/VariantZero/Audio/SW_Repair")),F(TEXT("/Game/VariantZero/Audio/SW_Hit"));
    Calm->SetSound(A.Object);Alert->SetSound(B.Object);Pulse=C.Object;Shield=D.Object;Repair=E.Object;Hit=F.Object;
    Calm->bAllowSpatialization=false;Alert->bAllowSpatialization=false;
}
void AVZAudioDirector::BeginPlay(){Super::BeginPlay();ApplyMix();Calm->Play();Alert->Play();Exploration->Play();}
void AVZAudioDirector::ApplyMix()
{
    const auto* PC=Cast<AVZPlayerController>(UGameplayStatics::GetPlayerController(this,0));
    const float Music=PC?PC->MusicVolume:1.f;
    Calm->SetVolumeMultiplier(.45f*(1-Mix)*(1-RegionMix)*Music);
    Exploration->SetVolumeMultiplier(.45f*(1-Mix)*RegionMix*Music);
    Alert->SetVolumeMultiplier(.4f*Mix*Music);
}
void AVZAudioDirector::Tick(float Dt)
{
    Super::Tick(Dt);auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(this,0));
    const bool Active=P&&IsValid(P->Disturbance)&&P->Disturbance->Active;
    Mix=FMath::FInterpTo(Mix,Active?1.f:0.f,Dt,1.2f);
    const bool InGarden=P&&P->Garden&&P->Garden->Enabled()&&P->GetActorLocation().Y>1800;
    RegionMix=FMath::FInterpTo(RegionMix,InGarden?1.f:0.f,Dt,.8f);
    ApplyMix();
}
void AVZAudioDirector::PlayCue(UWorld* World,FName Name)
{
    if(!World)return;
    const auto* PC=Cast<AVZPlayerController>(UGameplayStatics::GetPlayerController(World,0));
    const float Effects=PC?PC->EffectsVolume:1.f;if(Effects<=0)return;
    for(TActorIterator<AVZAudioDirector> It(World);It;++It)
    {
        if(Name==TEXT("Hit"))
        {
            const double Now=World->GetTimeSeconds();
            if(Now-It->LastHitTime<.08)return;
            It->LastHitTime=Now;
        }
        USoundWave* Sound=Name==TEXT("Pulse")?It->Pulse.Get():Name==TEXT("Shield")?It->Shield.Get():Name==TEXT("Repair")?It->Repair.Get():Name==TEXT("Hit")?It->Hit.Get():nullptr;
        if(Sound)UGameplayStatics::PlaySound2D(World,Sound,.65f*Effects);break;
    }
}
