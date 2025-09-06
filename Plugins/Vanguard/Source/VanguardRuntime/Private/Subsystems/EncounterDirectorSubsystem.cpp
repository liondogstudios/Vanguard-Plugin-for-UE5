#include "Subsystems/EncounterDirectorSubsystem.h"
#include "VanguardAPI.h"
#include "Data/EncounterArchetype.h"
#include "Components/EncounterHandleComponent.h"
#include "Engine/World.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/PlatformFilemanager.h"

UEncounterDirectorSubsystem::UEncounterDirectorSubsystem()
{
	// Set default values
	MaxBudget = 10;
	MinEncounterDistance = 1000.0f;
	MaxActiveEncounters = 5;
	MaxPerformanceBudgetMs = 2.0f;
	
	CurrentBudgetUsage = 0;
	LastKnownPlayerLocation = FVector::ZeroVector;
	PerformanceBudgetUsed = 0.0f;
}

void UEncounterDirectorSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogVanguard, Warning, TEXT("Encounter Director Subsystem initialized"));
	
	// Clear any existing data
	ActiveEncounters.Empty();
	ScheduledEncounters.Empty();
	LocationCooldowns.Empty();
	
	CurrentBudgetUsage = 0;
	PerformanceBudgetUsed = 0.0f;
}

void UEncounterDirectorSubsystem::Deinitialize()
{
	UE_LOG(LogVanguard, Warning, TEXT("Encounter Director Subsystem shutting down"));
	
	// Emergency cleanup of all active encounters
	EmergencyCleanup();
	
	// Clear all data
	ActiveEncounters.Empty();
	ScheduledEncounters.Empty();
	LocationCooldowns.Empty();
	
	Super::Deinitialize();
}

void UEncounterDirectorSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Track performance budget
	double StartTime = FPlatformTime::Seconds();
	PerformanceBudgetUsed = 0.0f;
	
	// Process scheduled encounters
	ProcessScheduledEncounters(DeltaTime);
	
	// Update active encounters
	if (CheckPerformanceBudget(StartTime))
	{
		UpdateActiveEncounters(DeltaTime);
	}
	
	// Clean up expired cooldowns
	if (CheckPerformanceBudget(StartTime))
	{
		CleanupExpiredCooldowns(GetWorld()->GetTimeSeconds());
	}
	
	// Calculate final performance budget usage
	PerformanceBudgetUsed = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
}

bool UEncounterDirectorSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Only create in game worlds, not in editor preview worlds
	if (UWorld* World = Cast<UWorld>(Outer))
	{
		return World->IsGameWorld();
	}
	return false;
}

bool UEncounterDirectorSubsystem::TryScheduleEncounter(const FGameplayTag& EncounterType, const FVector& NearLocation, FGuid& OutEncounterId)
{
	if (!EncounterType.IsValid())
	{
		UE_LOG(LogVanguard, Warning, TEXT("Invalid encounter type for scheduling"));
		return false;
	}
	
	// For now, create a simple scheduled encounter
	// In a full implementation, this would query Asset Manager for matching archetypes
	FScheduledEncounter NewScheduled;
	NewScheduled.TargetLocation = NearLocation;
	NewScheduled.ScheduledTime = GetWorld()->GetTimeSeconds() + 1.0f; // Schedule for 1 second from now
	
	ScheduledEncounters.Add(NewScheduled);
	
	UE_LOG(LogVanguard, Log, TEXT("Scheduled encounter of type %s near location %s"), 
		   *EncounterType.ToString(), *NearLocation.ToString());
	
	// Generate a temporary ID for tracking
	OutEncounterId = FGuid::NewGuid();
	return true;
}

bool UEncounterDirectorSubsystem::TryScheduleEncounterFromArchetype(UEncounterArchetype* Archetype, const FVector& NearLocation, FGuid& OutEncounterId)
{
	if (!Archetype)
	{
		UE_LOG(LogVanguard, Warning, TEXT("Null archetype provided for encounter scheduling"));
		return false;
	}
	
	// Check budget availability
	if (!IsBudgetAvailable(Archetype->BudgetCost))
	{
		UE_LOG(LogVanguard, Log, TEXT("Insufficient budget for encounter archetype %s (needs %d, have %d)"), 
			   *Archetype->GetName(), Archetype->BudgetCost, GetAvailableBudget());
		return false;
	}
	
	// Check if location is available
	if (!IsLocationAvailable(NearLocation))
	{
		UE_LOG(LogVanguard, Log, TEXT("Location %s not available for encounter"), *NearLocation.ToString());
		return false;
	}
	
	// Create scheduled encounter
	FScheduledEncounter NewScheduled;
	NewScheduled.Archetype = Archetype;
	NewScheduled.TargetLocation = NearLocation;
	NewScheduled.ScheduledTime = GetWorld()->GetTimeSeconds() + FMath::RandRange(1.0f, 5.0f);
	
	ScheduledEncounters.Add(NewScheduled);
	OutEncounterId = FGuid::NewGuid();
	
	UE_LOG(LogVanguard, Log, TEXT("Scheduled encounter from archetype %s near location %s"), 
		   *Archetype->GetName(), *NearLocation.ToString());
	
	return true;
}

bool UEncounterDirectorSubsystem::ForceSpawnEncounter(UEncounterArchetype* Archetype, const FVector& Location, FGuid& OutEncounterId)
{
	if (!Archetype)
	{
		return false;
	}
	
	// Create active encounter
	FActiveEncounter NewEncounter;
	NewEncounter.EncounterId = FGuid::NewGuid();
	NewEncounter.Archetype = Archetype;
	NewEncounter.Location = Location;
	NewEncounter.StartTime = GetWorld()->GetTimeSeconds();
	NewEncounter.State = FGameplayTag::RequestGameplayTag(TEXT("Encounter.State.Active"));
	NewEncounter.BudgetCost = Archetype->BudgetCost;
	NewEncounter.Priority = 1;
	
	// Add to active encounters
	ActiveEncounters.Add(NewEncounter.EncounterId, NewEncounter);
	CurrentBudgetUsage += NewEncounter.BudgetCost;
	
	// Set location cooldown
	SetLocationCooldown(Location, Archetype->CooldownDuration);
	
	OutEncounterId = NewEncounter.EncounterId;
	
	UE_LOG(LogVanguard, Log, TEXT("Force spawned encounter %s at location %s"), 
		   *Archetype->GetName(), *Location.ToString());
	
	return true;
}

void UEncounterDirectorSubsystem::EndEncounter(const FGuid& EncounterId, const FGameplayTag& EndReason)
{
	if (FActiveEncounter* Encounter = ActiveEncounters.Find(EncounterId))
	{
		UE_LOG(LogVanguard, Log, TEXT("Ending encounter %s for reason: %s"), 
			   *EncounterId.ToString(), *EndReason.ToString());
		
		// Cleanup encounter actors
		CleanupEncounterActors(*Encounter);
		
		// Update budget
		CurrentBudgetUsage -= Encounter->BudgetCost;
		CurrentBudgetUsage = FMath::Max(0, CurrentBudgetUsage);
		
		// Remove from active encounters
		ActiveEncounters.Remove(EncounterId);
	}
}

bool UEncounterDirectorSubsystem::GetActiveEncounter(const FGuid& EncounterId, FActiveEncounter& OutEncounter) const
{
	if (const FActiveEncounter* Encounter = ActiveEncounters.Find(EncounterId))
	{
		OutEncounter = *Encounter;
		return true;
	}
	return false;
}

TArray<FActiveEncounter> UEncounterDirectorSubsystem::GetAllActiveEncounters() const
{
	TArray<FActiveEncounter> Result;
	ActiveEncounters.GenerateValueArray(Result);
	return Result;
}

TArray<FActiveEncounter> UEncounterDirectorSubsystem::GetEncountersNearLocation(const FVector& Location, float Radius) const
{
	TArray<FActiveEncounter> Result;
	
	for (const auto& Pair : ActiveEncounters)
	{
		if (FVector::Dist(Pair.Value.Location, Location) <= Radius)
		{
			Result.Add(Pair.Value);
		}
	}
	
	return Result;
}

bool UEncounterDirectorSubsystem::IsLocationAvailable(const FVector& Location, float MinRadius) const
{
	// Check cooldown
	if (IsLocationOnCooldown(Location))
	{
		return false;
	}
	
	// Check distance to other active encounters
	float CheckRadius = MinRadius > 0.0f ? MinRadius : MinEncounterDistance;
	for (const auto& Pair : ActiveEncounters)
	{
		if (FVector::Dist(Pair.Value.Location, Location) < CheckRadius)
		{
			return false;
		}
	}
	
	return true;
}

void UEncounterDirectorSubsystem::SetLocationCooldown(const FVector& Location, float Duration)
{
	if (Duration > 0.0f)
	{
		FString LocationKey = GenerateLocationKey(Location);
		float CooldownEndTime = GetWorld()->GetTimeSeconds() + Duration;
		LocationCooldowns.Add(LocationKey, CooldownEndTime);
		
		UE_LOG(LogVanguard, VerboseLog, TEXT("Set location cooldown for %s until %f"), 
			   *LocationKey, CooldownEndTime);
	}
}

bool UEncounterDirectorSubsystem::IsLocationOnCooldown(const FVector& Location) const
{
	FString LocationKey = GenerateLocationKey(Location);
	if (const float* CooldownEndTime = LocationCooldowns.Find(LocationKey))
	{
		return GetWorld()->GetTimeSeconds() < *CooldownEndTime;
	}
	return false;
}

bool UEncounterDirectorSubsystem::IsBudgetAvailable(int32 RequiredBudget) const
{
	return (CurrentBudgetUsage + RequiredBudget) <= MaxBudget;
}

void UEncounterDirectorSubsystem::UpdatePlayerLocation(const FVector& NewLocation)
{
	LastKnownPlayerLocation = NewLocation;
}

void UEncounterDirectorSubsystem::EmergencyCleanup()
{
	UE_LOG(LogVanguard, Warning, TEXT("Performing emergency cleanup of %d active encounters"), 
		   ActiveEncounters.Num());
	
	// End all active encounters
	TArray<FGuid> EncounterIds;
	ActiveEncounters.GetKeys(EncounterIds);
	
	for (const FGuid& EncounterId : EncounterIds)
	{
		EndEncounter(EncounterId, FGameplayTag::RequestGameplayTag(TEXT("Encounter.End.Emergency")));
	}
	
	// Clear scheduled encounters
	ScheduledEncounters.Empty();
	
	// Reset budget
	CurrentBudgetUsage = 0;
}

void UEncounterDirectorSubsystem::ProcessScheduledEncounters(float DeltaTime)
{
	if (ScheduledEncounters.Num() == 0)
	{
		return;
	}
	
	float CurrentTime = GetWorld()->GetTimeSeconds();
	TArray<int32> IndicesToRemove;
	
	for (int32 i = 0; i < ScheduledEncounters.Num(); ++i)
	{
		FScheduledEncounter& Scheduled = ScheduledEncounters[i];
		
		if (CurrentTime >= Scheduled.ScheduledTime)
		{
			// Try to spawn the encounter
			FGuid SpawnedId;
			bool bSpawned = false;
			
			if (Scheduled.Archetype.IsValid())
			{
				if (UEncounterArchetype* Archetype = Scheduled.Archetype.LoadSynchronous())
				{
					bSpawned = ForceSpawnEncounter(Archetype, Scheduled.TargetLocation, SpawnedId);
				}
			}
			
			if (bSpawned)
			{
				UE_LOG(LogVanguard, Log, TEXT("Successfully spawned scheduled encounter"));
				IndicesToRemove.Add(i);
			}
			else
			{
				// Retry logic
				Scheduled.RetryAttempts++;
				if (Scheduled.RetryAttempts >= Scheduled.MaxRetries)
				{
					UE_LOG(LogVanguard, Warning, TEXT("Failed to spawn encounter after %d retries"), 
						   Scheduled.MaxRetries);
					IndicesToRemove.Add(i);
				}
				else
				{
					// Reschedule for later
					Scheduled.ScheduledTime = CurrentTime + FMath::RandRange(5.0f, 15.0f);
					UE_LOG(LogVanguard, Log, TEXT("Rescheduling encounter attempt %d/%d"), 
						   Scheduled.RetryAttempts, Scheduled.MaxRetries);
				}
			}
		}
	}
	
	// Remove processed encounters in reverse order
	for (int32 i = IndicesToRemove.Num() - 1; i >= 0; --i)
	{
		ScheduledEncounters.RemoveAt(IndicesToRemove[i]);
	}
}

void UEncounterDirectorSubsystem::UpdateActiveEncounters(float DeltaTime)
{
	TArray<FGuid> EncountersToEnd;
	
	for (auto& Pair : ActiveEncounters)
	{
		FActiveEncounter& Encounter = Pair.Value;
		
		// Clean up invalid participants
		Encounter.Participants.RemoveAll([](TWeakObjectPtr<AActor> Actor) {
			return !Actor.IsValid();
		});
		
		// Check if encounter should end (no valid participants)
		if (Encounter.Participants.Num() == 0)
		{
			EncountersToEnd.Add(Encounter.EncounterId);
		}
	}
	
	// End encounters that have no participants
	for (const FGuid& EncounterId : EncountersToEnd)
	{
		EndEncounter(EncounterId, FGameplayTag::RequestGameplayTag(TEXT("Encounter.End.NoParticipants")));
	}
}

void UEncounterDirectorSubsystem::CleanupExpiredCooldowns(float CurrentTime)
{
	TArray<FString> ExpiredKeys;
	
	for (const auto& Pair : LocationCooldowns)
	{
		if (CurrentTime >= Pair.Value)
		{
			ExpiredKeys.Add(Pair.Key);
		}
	}
	
	for (const FString& Key : ExpiredKeys)
	{
		LocationCooldowns.Remove(Key);
	}
	
	if (ExpiredKeys.Num() > 0)
	{
		UE_LOG(LogVanguard, VerboseLog, TEXT("Cleaned up %d expired location cooldowns"), ExpiredKeys.Num());
	}
}

bool UEncounterDirectorSubsystem::ValidateSpawnConditions(UEncounterArchetype* Archetype, const FVector& Location) const
{
	if (!Archetype)
	{
		return false;
	}
	
	// Check budget
	if (!IsBudgetAvailable(Archetype->BudgetCost))
	{
		return false;
	}
	
	// Check active encounter limit
	if (ActiveEncounters.Num() >= MaxActiveEncounters)
	{
		return false;
	}
	
	// Check location availability
	if (!IsLocationAvailable(Location))
	{
		return false;
	}
	
	return true;
}

bool UEncounterDirectorSubsystem::FindSpawnPointWithEQS(UEncounterArchetype* Archetype, const FVector& NearLocation, FVector& OutSpawnLocation)
{
	// This would use EQS to find appropriate spawn points
	// For now, just return a simple offset from the requested location
	OutSpawnLocation = NearLocation + FVector(FMath::RandRange(-500.0f, 500.0f), FMath::RandRange(-500.0f, 500.0f), 0.0f);
	return true;
}

bool UEncounterDirectorSubsystem::SpawnEncounterActors(const FActiveEncounter& Encounter)
{
	// This would spawn the actual actors for the encounter
	// Implementation would depend on the specific encounter archetype
	UE_LOG(LogVanguard, Log, TEXT("Spawning actors for encounter %s"), *Encounter.EncounterId.ToString());
	return true;
}

void UEncounterDirectorSubsystem::CleanupEncounterActors(const FActiveEncounter& Encounter)
{
	// Clean up all participants
	for (TWeakObjectPtr<AActor> Actor : Encounter.Participants)
	{
		if (Actor.IsValid())
		{
			// Remove encounter handle component
			if (UEncounterHandleComponent* HandleComponent = Actor->FindComponentByClass<UEncounterHandleComponent>())
			{
				HandleComponent->ClearEncounterHandle();
			}
			
			// Optionally destroy the actor or let it continue existing
			// Actor->Destroy();
		}
	}
	
	UE_LOG(LogVanguard, Log, TEXT("Cleaned up encounter actors for %s"), *Encounter.EncounterId.ToString());
}

FString UEncounterDirectorSubsystem::GenerateLocationKey(const FVector& Location) const
{
	// Generate a key based on quantized location (to handle slight position differences)
	int32 X = FMath::RoundToInt(Location.X / 100.0f) * 100;
	int32 Y = FMath::RoundToInt(Location.Y / 100.0f) * 100;
	int32 Z = FMath::RoundToInt(Location.Z / 100.0f) * 100;
	
	return FString::Printf(TEXT("%d_%d_%d"), X, Y, Z);
}

bool UEncounterDirectorSubsystem::CheckPerformanceBudget(float StartTime) const
{
	float ElapsedMs = (FPlatformTime::Seconds() - StartTime) * 1000.0f;
	return ElapsedMs < MaxPerformanceBudgetMs;
}