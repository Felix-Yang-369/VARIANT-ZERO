#include "VZSaveLocation.h"
#include "VZState.h"
#include "HAL/PlatformProcess.h"
#include "HAL/FileManager.h"
#include "Misc/Paths.h"
#include "Misc/FileHelper.h"

FString FVZSaveLocation::UserRoot(){return FPaths::ConvertRelativePathToFull(FString(FPlatformProcess::UserSettingsDir())/TEXT("VariantZero"));}
FString FVZSaveLocation::StoryDirectory(const FString& Root){return Root/TEXT("SaveGames/Story");}
EVZMigration FVZSaveLocation::ImportLegacy(const FString& Source,const FString& Destination)
{
    auto& Files=IFileManager::Get();
    const FString Target=FPaths::ConvertRelativePathToFull(Destination);
    if(Files.DirectoryExists(*Target)||Files.FileExists(*Target))return EVZMigration::AlreadyPresent;
    if(FPaths::IsRelative(Source))return EVZMigration::Failed;
    TMap<FString,FString> Copies;
    FString Report=TEXT("VariantZero legacy save import; originals retained.\nSource: ")+Source+TEXT("\n");
    for(int32 Slot=0;Slot<4;++Slot)
    {
        const FString Name=FString::Printf(TEXT("slot_%d.vzsave"),Slot),Path=Source/Name;
        FVZWorldState S;
        const auto Main=FVZSaveStore::ReadOne(Path,S),Backup=FVZSaveStore::ReadOne(Path+TEXT(".bak"),S);
        if(Main==EVZReadResult::Missing&&Backup==EVZReadResult::Missing)continue;
        if(Main==EVZReadResult::UnsupportedVersion||Backup==EVZReadResult::UnsupportedVersion||Main==EVZReadResult::IOError||Backup==EVZReadResult::IOError)return EVZMigration::Failed;
        if(Main!=EVZReadResult::Ok&&Backup!=EVZReadResult::Ok)return EVZMigration::Failed;
        Copies.Add(Name,Main==EVZReadResult::Ok?Path:Path+TEXT(".bak"));
        if(Backup==EVZReadResult::Ok)Copies.Add(Name+TEXT(".bak"),Path+TEXT(".bak"));
        Report+=Name+(Main==EVZReadResult::Ok?TEXT(": original\n"):TEXT(": recovered from backup\n"));
    }
    if(Copies.IsEmpty())return EVZMigration::NoSource;
    const FString Parent=FPaths::GetPath(Target);
    const FString Stage=Target+TEXT(".import-")+FGuid::NewGuid().ToString(EGuidFormats::Digits);
    // Only rename the newly created sibling staging directory. Never move a legacy tree.
    if(!FPaths::IsUnderDirectory(Stage,Parent)||!FPaths::IsUnderDirectory(Target,Parent)||!Files.MakeDirectory(*Stage,true))return EVZMigration::Failed;
    for(const auto& Copy:Copies)
    {
        const FString To=Stage/Copy.Key;
        if(Files.Copy(*To,*Copy.Value,false,false)!=COPY_OK)return EVZMigration::Failed;
        FVZWorldState Verified;if(FVZSaveStore::ReadOne(To,Verified)!=EVZReadResult::Ok)return EVZMigration::Failed;
        TArray<uint8> Before,After;
        if(!FFileHelper::LoadFileToArray(Before,*Copy.Value)||!FFileHelper::LoadFileToArray(After,*To)||Before!=After)return EVZMigration::Failed;
    }
    if(!FFileHelper::SaveStringToFile(Report,*(Stage/TEXT("migration.txt"))))return EVZMigration::Failed;
    if(Files.DirectoryExists(*Target)||Files.FileExists(*Target))return EVZMigration::AlreadyPresent;
    if(!Files.Move(*Target,*Stage,false,false,false,true))return EVZMigration::Failed;
    return EVZMigration::Imported;
}
