#pragma once
#include "CoreMinimal.h"
#include "InputCoreTypes.h"
class UPlayerInput;
struct FVZInputSpec { const TCHAR* Id; const TCHAR* Label; FKey Keyboard; FKey Pad; float Axis=0; };
class VARIANTZEROUE_API FVZInputSettings
{
public:
    static const TArray<FVZInputSpec>& Specs();
    TArray<FKey> Keyboard,Pad;
    FVZInputSettings();
    bool Assign(int32 Index,FKey Key,bool Gamepad,FString& Error);
    bool Load(const FString& Path);
    bool Save(const FString& Path) const;
    void Apply(UPlayerInput* Input) const;
    FString KeyName(const TCHAR* Id) const;
};
