#include "Data/RivalArchetype.h"
#include "VanguardAPI.h"

URivalArchetype::URivalArchetype()
{
	// Set default display name
	DisplayName = FText::FromString(TEXT("Unnamed Rival"));
}

FPrimaryAssetId URivalArchetype::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(FPrimaryAssetType(TEXT("RivalArchetype")), GetFName());
}