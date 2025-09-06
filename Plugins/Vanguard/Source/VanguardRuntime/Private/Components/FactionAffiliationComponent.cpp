#include "Components/FactionAffiliationComponent.h"
#include "VanguardAPI.h"
#include "Data/FactionArchetype.h"
#include "Engine/AssetManager.h"

UFactionAffiliationComponent::UFactionAffiliationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Default to neutral team
	TeamId = 0;
	PlayerReputation = 0.0f;
	bCanDefect = false;
	DefectionThreshold = 75.0f;
}

void UFactionAffiliationComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize team attitude settings
	UpdateTeamAttitude();
	
	UE_LOG(LogVanguard, Log, TEXT("Faction Affiliation Component initialized with Team ID %d, Faction: %s"), 
		   TeamId, *FactionTag.ToString());
}

FGenericTeamId UFactionAffiliationComponent::GetGenericTeamId() const
{
	return FGenericTeamId(TeamId);
}

void UFactionAffiliationComponent::SetGenericTeamId(const FGenericTeamId& TeamID)
{
	TeamId = TeamID.GetId();
	UpdateTeamAttitude();
}

void UFactionAffiliationComponent::SetTeamId(uint8 NewTeamId)
{
	if (TeamId != NewTeamId)
	{
		uint8 OldTeamId = TeamId;
		TeamId = NewTeamId;
		UpdateTeamAttitude();
		
		UE_LOG(LogVanguard, Log, TEXT("Team ID changed from %d to %d for faction %s"), 
			   OldTeamId, NewTeamId, *FactionTag.ToString());
	}
}

void UFactionAffiliationComponent::SetFactionTag(const FGameplayTag& NewFactionTag)
{
	if (FactionTag != NewFactionTag)
	{
		FGameplayTag OldFactionTag = FactionTag;
		FactionTag = NewFactionTag;
		
		// Add to faction tags container
		FactionTags.AddTag(NewFactionTag);
		if (OldFactionTag.IsValid())
		{
			FactionTags.RemoveTag(OldFactionTag);
		}
		
		UpdateTeamAttitude();
		OnFactionChanged(OldFactionTag, NewFactionTag);
		
		UE_LOG(LogVanguard, Log, TEXT("Faction changed from %s to %s"), 
			   *OldFactionTag.ToString(), *NewFactionTag.ToString());
	}
}

void UFactionAffiliationComponent::ModifyPlayerReputation(float Delta)
{
	PlayerReputation = FMath::Clamp(PlayerReputation + Delta, -100.0f, 100.0f);
	
	// Check for potential defection
	if (bCanDefect && FMath::Abs(PlayerReputation) >= DefectionThreshold)
	{
		// Implementation for defection logic would go here
		UE_LOG(LogVanguard, Log, TEXT("Faction %s reputation with player is %f - considering defection"), 
			   *FactionTag.ToString(), PlayerReputation);
	}
	
	UE_LOG(LogVanguard, Log, TEXT("Modified player reputation for faction %s by %f, new value: %f"), 
		   *FactionTag.ToString(), Delta, PlayerReputation);
}

bool UFactionAffiliationComponent::IsHostileToTeam(uint8 OtherTeamId) const
{
	// Basic hostility check - non-neutral teams are hostile to each other
	if (TeamId == 0 || OtherTeamId == 0)
	{
		return false; // Neutral teams are not hostile
	}
	
	return TeamId != OtherTeamId;
}

bool UFactionAffiliationComponent::IsHostileToFaction(const FGameplayTag& OtherFactionTag) const
{
	if (!FactionTag.IsValid() || !OtherFactionTag.IsValid())
	{
		return false;
	}
	
	// Check faction archetype for hostility matrix
	if (UFactionArchetype* FactionArchetype = GetFactionArchetype())
	{
		float HostilityLevel = FactionArchetype->GetHostilityTowards(OtherFactionTag);
		return HostilityLevel > 0.0f; // Positive values indicate hostility
	}
	
	// Default behavior - different factions are potentially hostile
	return FactionTag != OtherFactionTag;
}

bool UFactionAffiliationComponent::TryDefectToFaction(const FGameplayTag& NewFactionTag)
{
	if (!bCanDefect || !NewFactionTag.IsValid() || NewFactionTag == FactionTag)
	{
		return false;
	}
	
	// Check defection conditions
	if (FMath::Abs(PlayerReputation) >= DefectionThreshold)
	{
		SetFactionTag(NewFactionTag);
		
		// Reset reputation to neutral with new faction
		PlayerReputation = 0.0f;
		
		UE_LOG(LogVanguard, Log, TEXT("Successfully defected to faction %s"), *NewFactionTag.ToString());
		return true;
	}
	
	return false;
}

UFactionArchetype* UFactionAffiliationComponent::GetFactionArchetype() const
{
	if (!FactionTag.IsValid())
	{
		return nullptr;
	}
	
	// This would use Asset Manager to load the faction archetype
	// For now, return nullptr as the full asset loading system isn't implemented
	return nullptr;
}

void UFactionAffiliationComponent::UpdateTeamAttitude()
{
	// Update AI perception team attitude settings
	// This would integrate with UE's team attitude solver
	UE_LOG(LogVanguard, VerboseLog, TEXT("Updated team attitude for Team ID %d"), TeamId);
}