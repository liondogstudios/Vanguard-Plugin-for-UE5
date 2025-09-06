#include "Data/EncounterArchetype.h"
#include "VanguardAPI.h"

UEncounterArchetype::UEncounterArchetype()
{
	// Set default values
	BudgetCost = 1;
	CooldownDuration = 300.0f; // 5 minutes default
}

FPrimaryAssetId UEncounterArchetype::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("EncounterArchetype")), GetFName());
}