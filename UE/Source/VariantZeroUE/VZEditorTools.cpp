#include "VZEditorTools.h"
#include "Animation/AnimSequence.h"
#include "Animation/Skeleton.h"
#if WITH_EDITOR
#include "ActorFactories/ActorFactoryBoxVolume.h"
#include "NavMesh/NavMeshBoundsVolume.h"
#include "NavigationSystem.h"
#include "Engine/World.h"
#include "Builders/CubeBuilder.h"
#include "NavigationData.h"
#include "NavigationPath.h"
#include "EngineUtils.h"
#endif
bool UVZEditorTools::AddNavigationBounds(UWorld* World)
{
#if WITH_EDITOR
    if (!World || World->IsGameWorld()) return false;
    auto* Bounds = World->SpawnActor<ANavMeshBoundsVolume>(FVector(0,0,300),FRotator::ZeroRotator);
    if (!Bounds) return false;
    auto* Builder = NewObject<UCubeBuilder>();
    Builder->X = 3200; Builder->Y = 2600; Builder->Z = 1000;
    UActorFactory::CreateBrushForVolumeActor(Bounds,Builder);
    Bounds->SetActorLabel(TEXT("VZ_Conservatory_Navigation"));
    if (auto* Nav = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World)) Nav->OnNavigationBoundsUpdated(Bounds);
    return Bounds->GetComponentsBoundingBox(true).GetSize().X > 1000;
#else
    return false;
#endif
}

bool UVZEditorTools::RebuildSceneNavigation(UWorld* World)
{
#if WITH_EDITOR
    if(!World || World->IsGameWorld()) return false;
    auto* Nav=FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
    if(!Nav) return false;
    Nav->ReleaseInitialBuildingLock();Nav->Build();
    for(TActorIterator<ANavigationData> It(World);It;++It) It->EnsureBuildCompletion();
    auto* Path=UNavigationSystemV1::FindPathToLocationSynchronously(World,FVector(-350,640,90),FVector(350,640,90));
    return Path && Path->IsValid() && !Path->IsPartial() && Path->PathPoints.Num()>2;
#else
    return false;
#endif
}

bool UVZEditorTools::AssignAnimationSkeleton(UAnimSequence* Animation, USkeleton* Skeleton)
{
#if WITH_EDITOR
    if (!Animation || !Skeleton) return false;
    Animation->SetSkeleton(Skeleton); Animation->MarkPackageDirty();
    return Animation->GetSkeleton() == Skeleton;
#else
    return false;
#endif
}
