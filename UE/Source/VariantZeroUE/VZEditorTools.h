#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "VZEditorTools.generated.h"
class UAnimSequence;
class USkeleton;
UCLASS()
class VARIANTZEROUE_API UVZEditorTools : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()
public:
    UFUNCTION(BlueprintCallable, Category="VariantZero|Editor")
    static bool AddNavigationBounds(UWorld* World);
    UFUNCTION(BlueprintCallable, Category="VariantZero|Editor")
    static bool AssignAnimationSkeleton(UAnimSequence* Animation, USkeleton* Skeleton);
    UFUNCTION(BlueprintCallable, Category="VariantZero|Editor")
    static bool RebuildSceneNavigation(UWorld* World);
};
