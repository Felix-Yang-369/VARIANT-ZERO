#include "../VZInputSettings.h"
#include "Misc/AutomationTest.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/Paths.h"
#include "HAL/FileManager.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVZInputTest,"VariantZero.Input.PersistenceAndConflicts",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVZInputTest::RunTest(const FString&)
{
    FVZInputSettings Layout;FString Error;
    TestFalse(TEXT("cannot steal forward key"),Layout.Assign(9,EKeys::W,false,Error));
    TestFalse(TEXT("keep Escape available"),Layout.Assign(9,EKeys::Escape,false,Error));
    TestFalse(TEXT("axis cannot become digital action"),Layout.Assign(9,EKeys::Gamepad_LeftX,true,Error));
    TestTrue(TEXT("keyboard rebinding"),Layout.Assign(9,EKeys::F,false,Error));
    TestTrue(TEXT("gamepad rebinding"),Layout.Assign(9,EKeys::Gamepad_DPad_Up,true,Error));
    const FString Dir=FPaths::ProjectSavedDir()/TEXT("Automation/Input")/FGuid::NewGuid().ToString();
    const FString Path=Dir/TEXT("settings.ini");
    IFileManager::Get().MakeDirectory(*Dir,true);FConfigFile Existing;Existing.SetFloat(TEXT("Audio"),TEXT("MasterVolume"),.4f);Existing.Write(Path);
    TestTrue(TEXT("save remapping"),Layout.Save(Path));FVZInputSettings Reloaded;TestTrue(TEXT("reload valid layout"),Reloaded.Load(Path));
    TestEqual(TEXT("pulse key persisted"),Reloaded.Keyboard[9],EKeys::F);TestEqual(TEXT("pad key persisted"),Reloaded.Pad[9],EKeys::Gamepad_DPad_Up);
    FConfigFile ReadBack;ReadBack.Read(Path);float Volume=0;ReadBack.GetFloat(TEXT("Audio"),TEXT("MasterVolume"),Volume);TestEqual(TEXT("audio setting preserved"),Volume,.4f);
    ReadBack.SetString(TEXT("Bindings"),TEXT("Keyboard9"),TEXT("W"));ReadBack.Write(Path);
    TestFalse(TEXT("corrupt duplicate layout rejected"),Reloaded.Load(Path));TestEqual(TEXT("failed load leaves previous layout"),Reloaded.Keyboard[9],EKeys::F);
    ReadBack.SetString(TEXT("Bindings"),TEXT("Version"),TEXT("999"));ReadBack.Write(Path);TestFalse(TEXT("future layout rejected"),Reloaded.Load(Path));TestFalse(TEXT("future layout not overwritten"),Reloaded.Save(Path));
    FConfigFile Legacy;
    Legacy.SetString(TEXT("Bindings"),TEXT("Version"),TEXT("1"));
    Legacy.SetString(TEXT("Bindings"),TEXT("Keyboard9"),TEXT("Two"));
    Legacy.SetString(TEXT("Bindings"),TEXT("Pad9"),TEXT("Gamepad_DPad_Right"));
    const FString LegacyPath=Dir/TEXT("legacy.ini");Legacy.Write(LegacyPath);
    TestTrue(TEXT("migrate old user keys conflicting with new defaults"),Reloaded.Load(LegacyPath));
    TestEqual(TEXT("old keyboard selection kept"),Reloaded.Keyboard[9],EKeys::Two);
    TestEqual(TEXT("old pad selection kept"),Reloaded.Pad[9],EKeys::Gamepad_DPad_Right);
    TestTrue(TEXT("new keyboard skill assigned unused key"),Reloaded.Keyboard[15]!=EKeys::Two);
    TestTrue(TEXT("new pad skill assigned unused key"),Reloaded.Pad[16]!=EKeys::Gamepad_DPad_Right);
    TestTrue(TEXT("migrated layout persists"),Reloaded.Save(LegacyPath));
    TestTrue(TEXT("version two reopens"),Layout.Load(LegacyPath));
    IFileManager::Get().DeleteDirectory(*Dir,false,true);return true;
}
#endif
