#include "VZPrototype.h"
#include "VZCombat.h"
#include "VZState.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Misc/CommandLine.h"
#include "Camera/CameraActor.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "UnrealClient.h"
#include "NavigationSystem.h"

void StartVZOrderVerification(UWorld* World)
{
#if !UE_BUILD_SHIPPING
    if(!FParse::Param(FCommandLine::Get(),TEXT("VZOrderTest")))return;
    if(World->GetTimeSeconds()<1||UNavigationSystemV1::IsNavigationBeingBuiltOrLocked(World))
    {
        if(World->GetTimeSeconds()>10){UE_LOG(LogTemp,Error,TEXT("VZ_ORDERS: FAIL navigation did not become ready"));FPlatformMisc::RequestExitWithStatus(true,1);return;}
        FTimerHandle Ready;World->GetTimerManager().SetTimer(Ready,[World](){StartVZOrderVerification(World);},.25f,false);return;
    }
    auto Ok=MakeShared<bool>(true);auto Id=MakeShared<FGuid>();auto Before=MakeShared<float>(0);
    auto Check=[Ok](bool Result,const TCHAR* What){*Ok &=Result;if(!Result)UE_LOG(LogTemp,Error,TEXT("VZ_ORDER_CHECK: %s"),What);};
    auto Later=[World](float Seconds,TFunction<void()> Fn){FTimerHandle H;World->GetTimerManager().SetTimer(H,MoveTemp(Fn),Seconds,false);};
    Later(1,[World,Id,Check]()
    {
        auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(World,0));auto* C=P->Companions[0].Get();
        *Id=P->State()->State.Companions[0].IndividualId;
        P->SetActorLocation(FVector(-550,400,95));C->SetActorLocation(FVector(-350,640,40));
        Check(!P->IssueGroundOrder(FVector(9000,9000,0)),TEXT("reject distant order"));
        Check(!P->IssueGroundOrder(FVector(350,640,800)),TEXT("reject unsupported surface"));
        Check(P->IssueGroundOrder(FVector(350,640,0)),TEXT("accept full path around planter"));
        P->RecallParty();Check(!C->HasGroundOrder&&P->CompanionSkillCooldown==0,TEXT("recall cancels without skill cost"));
        C->Vital->Health=0;Check(!P->IssueGroundOrder(FVector(350,640,0)),TEXT("down companion rejects order"));C->Vital->Restore();
        Check(P->IssueGroundOrder(FVector(350,640,0)),TEXT("restart detour"));
    });
    Later(10,[World,Check,Before]()
    {
        auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(World,0));auto* C=P->Companions[0].Get();
        Check(C->GroundSkillsCompleted==1&&!C->HasGroundOrder,TEXT("actually navigate and execute once"));
        P->SetActorLocation(FVector(100,-450,95));C->SetActorLocation(FVector(150,-450,40));
        P->CompanionSkillCooldown=0;P->Vital->InvulnerableSeconds=30;P->StartEncounter();
        *Before=P->Disturbance->Vital->Health;
        Check(P->IssueGroundOrder(FVector(400,-450,0)),TEXT("combat area order accepted"));
        auto* Camera=World->SpawnActor<ACameraActor>(FVector(-850,-1200,650),FRotator::ZeroRotator);
        Camera->SetActorRotation((FVector(100,-150,180)-Camera->GetActorLocation()).Rotation());Camera->GetCameraComponent()->SetFieldOfView(65);
        UGameplayStatics::GetPlayerController(World,0)->SetViewTarget(Camera);
    });
    Later(10.25f,[](){FScreenshotRequest::RequestScreenshot(TEXT("VariantZero_OrderPreview.png"),false,false);});
    Later(14,[World,Check,Before,Id,Ok]()
    {
        auto* P=Cast<AVZEcologist>(UGameplayStatics::GetPlayerCharacter(World,0));auto* C=P->Companions[0].Get();
        Check(C->GroundSkillsCompleted==2,TEXT("area skill executes once"));
        Check(*Before-P->Disturbance->Vital->Health>=32,TEXT("area skill hits node"));
        Check(P->CompanionSkillCooldown>0,TEXT("cooldown starts at execution"));
        Check(!P->IssueGroundOrder(FVector(400,-450,0)),TEXT("cooldown rejects order"));
        P->RecallParty();Check(!C->HasGroundOrder&&C->RecallSeconds>0,TEXT("recall suppresses auto attack temporarily"));
        P->Respawn();Check(!P->AimingOrder&&P->State()->State.Companions[0].IndividualId==*Id,TEXT("respawn keeps identity and clears commands"));
        UE_LOG(LogTemp,Display,TEXT("VZ_ORDERS: %s ground validation, recall, down state, actual detour, area execution, cooldown, identity"),*Ok?TEXT("PASS"):TEXT("FAIL"));
        FPlatformMisc::RequestExitWithStatus(true,*Ok?0:1);
    });
#endif
}
