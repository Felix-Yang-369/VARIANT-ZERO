#include "../VZCombat.h"
#include "Misc/AutomationTest.h"
#if WITH_DEV_AUTOMATION_TESTS
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVZDamageTest,"VariantZero.Combat.DamageBoundaries",EAutomationTestFlags::EditorContext|EAutomationTestFlags::EngineFilter)
bool FVZDamageTest::RunTest(const FString& Parameters)
{
    auto* Vital=NewObject<UVZVitalComponent>();
    TestEqual(TEXT("negative damage rejected"),Vital->ReceiveDamage(-10),0.f);
    Vital->ShieldSeconds=3;TestEqual(TEXT("shield absorbs 80 percent"),Vital->ReceiveDamage(50),10.f);
    Vital->InvulnerableSeconds=.3f;TestEqual(TEXT("dodge invulnerability"),Vital->ReceiveDamage(100),0.f);
    Vital->InvulnerableSeconds=0;Vital->ShieldSeconds=0;TestEqual(TEXT("overkill clamped to current health"),Vital->ReceiveDamage(999),90.f);
    TestTrue(TEXT("downed"),Vital->IsDown());TestEqual(TEXT("cannot damage downed actor"),Vital->ReceiveDamage(5),0.f);
    Vital->Restore();TestEqual(TEXT("revive restores health"),Vital->Health,100.f);return true;
}
#endif
