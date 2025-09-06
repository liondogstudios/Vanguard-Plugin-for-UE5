#include "Data/TraitArchetype.h"
#include "VanguardAPI.h"

UTraitArchetype::UTraitArchetype()
{
	TraitName = FText::FromString(TEXT("Unnamed Trait"));
	Description = FText::FromString(TEXT("No description available"));
	AcquisitionWeight = 1.0f;
	MaxStacks = 1;
	bPersistsThroughDeath = true;
}

FPrimaryAssetId UTraitArchetype::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("TraitArchetype")), GetFName());
}

bool UTraitArchetype::IsCompatibleWith(const FGameplayTagContainer& ExistingTraits) const
{
	// Check if any incompatible traits are present
	return !ExistingTraits.HasAny(IncompatibleTraits);
}

bool UTraitArchetype::ArePrerequisitesMet(const FGameplayTagContainer& ExistingTraits) const
{
	// Check if all prerequisite traits are present
	return ExistingTraits.HasAll(PrerequisiteTraits);
}