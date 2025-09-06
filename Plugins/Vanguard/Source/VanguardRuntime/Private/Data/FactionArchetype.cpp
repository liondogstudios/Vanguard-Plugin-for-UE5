#include "Data/FactionArchetype.h"
#include "VanguardAPI.h"

UFactionArchetype::UFactionArchetype()
{
	FactionName = FText::FromString(TEXT("Unnamed Faction"));
	TeamId = 0; // Neutral by default
	PrimaryColor = FLinearColor::White;
	SecondaryColor = FLinearColor::Gray;
}

FPrimaryAssetId UFactionArchetype::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("FactionArchetype")), GetFName());
}

float UFactionArchetype::GetHostilityTowards(const FGameplayTag& OtherFactionTag) const
{
	if (const float* HostilityPtr = HostilityMatrix.Find(OtherFactionTag))
	{
		return *HostilityPtr;
	}
	return 0.0f; // Neutral by default
}

void UFactionArchetype::SetHostilityTowards(const FGameplayTag& OtherFactionTag, float HostilityLevel)
{
	// Clamp hostility to valid range
	HostilityLevel = FMath::Clamp(HostilityLevel, -1.0f, 1.0f);
	HostilityMatrix.Add(OtherFactionTag, HostilityLevel);
}