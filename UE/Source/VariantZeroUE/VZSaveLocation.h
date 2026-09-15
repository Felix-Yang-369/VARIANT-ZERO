#pragma once
#include "CoreMinimal.h"
enum class EVZMigration { NoSource, AlreadyPresent, Imported, Failed };
class VARIANTZEROUE_API FVZSaveLocation
{
public:
    static FString UserRoot();
    static FString StoryDirectory(const FString& UserRoot);
    static EVZMigration ImportLegacy(const FString& Source,const FString& Destination);
};
