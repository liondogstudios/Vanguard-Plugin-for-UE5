#include "Components/EncounterHandleComponent.h"
#include "VanguardAPI.h"
#include "Engine/World.h"

UEncounterHandleComponent::UEncounterHandleComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Initialize default values
	SquadIndex = INDEX_NONE;
	Priority = 1;
	bIsEssential = false;
	bCanRetreat = true;
	AggressionLevel = 0.5f;
	EncounterJoinTime = 0.0f;
}

void UEncounterHandleComponent::BeginPlay()
{
	Super::BeginPlay();
	
	UE_LOG(LogVanguard, Log, TEXT("Encounter Handle Component initialized for actor %s"), 
		   GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
}

void UEncounterHandleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// Clean up encounter association
	if (IsInEncounter())
	{
		ClearEncounterHandle();
	}
	
	Super::EndPlay(EndPlayReason);
}

void UEncounterHandleComponent::InitializeEncounterHandle(const FGuid& InEncounterId, int32 InSquadIndex, const FGameplayTag& InSquadRole)
{
	EncounterId = InEncounterId;
	SquadIndex = InSquadIndex;
	SquadRole = InSquadRole;
	
	// Record join time
	if (UWorld* World = GetWorld())
	{
		EncounterJoinTime = World->GetTimeSeconds();
	}
	
	// Set initial state
	SetEncounterState(FGameplayTag::RequestGameplayTag(TEXT("Encounter.State.Active")));
	
	UE_LOG(LogVanguard, Log, TEXT("Initialized encounter handle for actor %s: Encounter %s, Squad %d, Role %s"), 
		   GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
		   *EncounterId.ToString(), SquadIndex, *SquadRole.ToString());
}

void UEncounterHandleComponent::ClearEncounterHandle()
{
	if (IsInEncounter())
	{
		UE_LOG(LogVanguard, Log, TEXT("Clearing encounter handle for actor %s from encounter %s"), 
			   GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
			   *EncounterId.ToString());
		
		// Clear encounter data
		EncounterId.Invalidate();
		SquadIndex = INDEX_NONE;
		SquadRole = FGameplayTag::EmptyTag;
		EncounterState = FGameplayTag::EmptyTag;
		TacticalTags.Reset();
		EncounterJoinTime = 0.0f;
		
		// Reset behavioral state
		AggressionLevel = 0.5f;
	}
}

void UEncounterHandleComponent::SetEncounterState(const FGameplayTag& NewState)
{
	if (NewState != EncounterState)
	{
		FGameplayTag OldState = EncounterState;
		EncounterState = NewState;
		
		OnEncounterStateChanged(OldState, NewState);
		
		UE_LOG(LogVanguard, Log, TEXT("Encounter state changed for actor %s: %s -> %s"), 
			   GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
			   *OldState.ToString(), *NewState.ToString());
	}
}

void UEncounterHandleComponent::SetAggressionLevel(float NewLevel)
{
	NewLevel = FMath::Clamp(NewLevel, 0.0f, 1.0f);
	
	if (!FMath::IsNearlyEqual(AggressionLevel, NewLevel))
	{
		float OldLevel = AggressionLevel;
		AggressionLevel = NewLevel;
		
		OnAggressionLevelChanged(OldLevel, NewLevel);
		
		UE_LOG(LogVanguard, Log, TEXT("Aggression level changed for actor %s: %f -> %f"), 
			   GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
			   OldLevel, NewLevel);
	}
}

void UEncounterHandleComponent::AddTacticalTag(const FGameplayTag& TacticalTag)
{
	if (TacticalTag.IsValid())
	{
		TacticalTags.AddTag(TacticalTag);
		UE_LOG(LogVanguard, Log, TEXT("Added tactical tag %s to actor %s"), 
			   *TacticalTag.ToString(), GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
	}
}

void UEncounterHandleComponent::RemoveTacticalTag(const FGameplayTag& TacticalTag)
{
	if (TacticalTag.IsValid())
	{
		TacticalTags.RemoveTag(TacticalTag);
		UE_LOG(LogVanguard, Log, TEXT("Removed tactical tag %s from actor %s"), 
			   *TacticalTag.ToString(), GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"));
	}
}

bool UEncounterHandleComponent::HasTacticalTag(const FGameplayTag& TacticalTag) const
{
	return TacticalTag.IsValid() && TacticalTags.HasTagExact(TacticalTag);
}

bool UEncounterHandleComponent::RequestRetreat()
{
	if (!bCanRetreat || !IsInEncounter())
	{
		return false;
	}
	
	// Set retreat state
	SetEncounterState(FGameplayTag::RequestGameplayTag(TEXT("Encounter.State.Retreating")));
	
	UE_LOG(LogVanguard, Log, TEXT("Actor %s requested retreat from encounter %s"), 
		   GetOwner() ? *GetOwner()->GetName() : TEXT("Unknown"),
		   *EncounterId.ToString());
	
	return true;
}

float UEncounterHandleComponent::GetEncounterDuration() const
{
	if (!IsInEncounter() || EncounterJoinTime <= 0.0f)
	{
		return 0.0f;
	}
	
	if (UWorld* World = GetWorld())
	{
		return World->GetTimeSeconds() - EncounterJoinTime;
	}
	
	return 0.0f;
}

bool UEncounterHandleComponent::IsInEncounter() const
{
	return EncounterId.IsValid() && SquadIndex != INDEX_NONE;
}

TArray<AActor*> UEncounterHandleComponent::GetSquadMembers() const
{
	TArray<AActor*> SquadMembers;
	
	if (!IsInEncounter())
	{
		return SquadMembers;
	}
	
	// This would query the encounter director for other actors in the same squad
	// For now, return empty array as the full encounter system isn't implemented yet
	
	return SquadMembers;
}