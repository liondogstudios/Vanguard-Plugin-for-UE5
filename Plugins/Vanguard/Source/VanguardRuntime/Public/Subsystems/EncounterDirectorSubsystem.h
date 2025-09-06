#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "GameplayTagContainer.h"
#include "Data/EncounterArchetype.h"
#include "EncounterDirectorSubsystem.generated.h"

class UEnvQuery;
class UEncounterArchetype;

/** Structure to hold active encounter data */
USTRUCT(BlueprintType)
struct VANGUARD_API FActiveEncounter
{
	GENERATED_BODY()

	/** Unique identifier for this encounter */
	UPROPERTY(BlueprintReadOnly)
	FGuid EncounterId;

	/** Archetype that defines this encounter */
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UEncounterArchetype> Archetype;

	/** Location where encounter is taking place */
	UPROPERTY(BlueprintReadOnly)
	FVector Location;

	/** Actors participating in this encounter */
	UPROPERTY(BlueprintReadOnly)
	TArray<TWeakObjectPtr<AActor>> Participants;

	/** Time when encounter started */
	UPROPERTY(BlueprintReadOnly)
	float StartTime = 0.0f;

	/** Current state of the encounter */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag State;

	/** Budget cost of this encounter */
	UPROPERTY(BlueprintReadOnly)
	int32 BudgetCost = 1;

	/** Priority level for resource allocation */
	UPROPERTY(BlueprintReadOnly)
	int32 Priority = 1;

	FActiveEncounter()
	{
		EncounterId = FGuid::NewGuid();
		Location = FVector::ZeroVector;
		StartTime = 0.0f;
		BudgetCost = 1;
		Priority = 1;
	}
};

/** Structure to hold encounter scheduling data */
USTRUCT(BlueprintType)
struct VANGUARD_API FScheduledEncounter
{
	GENERATED_BODY()

	/** Encounter archetype to spawn */
	UPROPERTY(BlueprintReadOnly)
	TSoftObjectPtr<UEncounterArchetype> Archetype;

	/** Desired spawn location */
	UPROPERTY(BlueprintReadOnly)
	FVector TargetLocation;

	/** Desired spawn time */
	UPROPERTY(BlueprintReadOnly)
	float ScheduledTime = 0.0f;

	/** Number of retry attempts if spawn fails */
	UPROPERTY(BlueprintReadOnly)
	int32 RetryAttempts = 0;

	/** Maximum retry attempts allowed */
	UPROPERTY(BlueprintReadOnly)
	int32 MaxRetries = 3;

	FScheduledEncounter()
	{
		TargetLocation = FVector::ZeroVector;
		ScheduledTime = 0.0f;
		RetryAttempts = 0;
		MaxRetries = 3;
	}
};

/**
 * Per-world orchestrator for encounter scheduling, budget management, and spatial queries
 * Handles streaming awareness and fail-safe cleanup
 */
UCLASS(BlueprintType)
class VANGUARD_API UEncounterDirectorSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	UEncounterDirectorSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	/** Currently active encounters */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Encounters")
	TMap<FGuid, FActiveEncounter> ActiveEncounters;

	/** Encounters scheduled for future spawning */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Scheduling")
	TArray<FScheduledEncounter> ScheduledEncounters;

	/** Current budget usage */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Budget")
	int32 CurrentBudgetUsage = 0;

	/** Maximum budget available */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Budget")
	int32 MaxBudget = 10;

	/** Minimum distance between encounters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spacing")
	float MinEncounterDistance = 1000.0f;

	/** Maximum number of active encounters */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Limits")
	int32 MaxActiveEncounters = 5;

	/** Cooldown locations and their expiry times */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Cooldowns")
	TMap<FString, float> LocationCooldowns;

	/** Player location tracking for proximity checks */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player")
	FVector LastKnownPlayerLocation;

	/** Time-sliced operations tracking */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	float PerformanceBudgetUsed = 0.0f;

	/** Maximum performance budget per frame (milliseconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance")
	float MaxPerformanceBudgetMs = 2.0f;

public:
	/** Try to schedule an encounter of the specified type */
	UFUNCTION(BlueprintCallable, Category = "Encounters")
	bool TryScheduleEncounter(const FGameplayTag& EncounterType, const FVector& NearLocation, FGuid& OutEncounterId);

	/** Try to schedule an encounter from a specific archetype */
	UFUNCTION(BlueprintCallable, Category = "Encounters")
	bool TryScheduleEncounterFromArchetype(UEncounterArchetype* Archetype, const FVector& NearLocation, FGuid& OutEncounterId);

	/** Force spawn an encounter immediately */
	UFUNCTION(BlueprintCallable, Category = "Encounters")
	bool ForceSpawnEncounter(UEncounterArchetype* Archetype, const FVector& Location, FGuid& OutEncounterId);

	/** End an active encounter */
	UFUNCTION(BlueprintCallable, Category = "Encounters")
	void EndEncounter(const FGuid& EncounterId, const FGameplayTag& EndReason);

	/** Get active encounter by ID */
	UFUNCTION(BlueprintCallable, Category = "Encounters")
	bool GetActiveEncounter(const FGuid& EncounterId, FActiveEncounter& OutEncounter) const;

	/** Get all active encounters */
	UFUNCTION(BlueprintCallable, Category = "Encounters")
	TArray<FActiveEncounter> GetAllActiveEncounters() const;

	/** Get encounters near a location */
	UFUNCTION(BlueprintCallable, Category = "Encounters")
	TArray<FActiveEncounter> GetEncountersNearLocation(const FVector& Location, float Radius) const;

	/** Check if location is available for encounters */
	UFUNCTION(BlueprintCallable, Category = "Spacing")
	bool IsLocationAvailable(const FVector& Location, float MinRadius = 0.0f) const;

	/** Set location cooldown */
	UFUNCTION(BlueprintCallable, Category = "Cooldowns")
	void SetLocationCooldown(const FVector& Location, float Duration);

	/** Check if location is on cooldown */
	UFUNCTION(BlueprintCallable, Category = "Cooldowns")
	bool IsLocationOnCooldown(const FVector& Location) const;

	/** Get current budget usage */
	UFUNCTION(BlueprintCallable, Category = "Budget")
	int32 GetCurrentBudgetUsage() const { return CurrentBudgetUsage; }

	/** Get available budget */
	UFUNCTION(BlueprintCallable, Category = "Budget")
	int32 GetAvailableBudget() const { return MaxBudget - CurrentBudgetUsage; }

	/** Check if budget is available for an encounter */
	UFUNCTION(BlueprintCallable, Category = "Budget")
	bool IsBudgetAvailable(int32 RequiredBudget) const;

	/** Update player location for proximity tracking */
	UFUNCTION(BlueprintCallable, Category = "Player")
	void UpdatePlayerLocation(const FVector& NewLocation);

	/** Get number of active encounters */
	UFUNCTION(BlueprintCallable, Category = "Encounters")
	int32 GetActiveEncounterCount() const { return ActiveEncounters.Num(); }

	/** Emergency cleanup of all encounters */
	UFUNCTION(BlueprintCallable, Category = "System")
	void EmergencyCleanup();

protected:
	/** Process scheduled encounters */
	void ProcessScheduledEncounters(float DeltaTime);

	/** Update active encounters */
	void UpdateActiveEncounters(float DeltaTime);

	/** Clean up expired cooldowns */
	void CleanupExpiredCooldowns(float CurrentTime);

	/** Validate encounter spawning conditions */
	bool ValidateSpawnConditions(UEncounterArchetype* Archetype, const FVector& Location) const;

	/** Find spawn point using EQS */
	bool FindSpawnPointWithEQS(UEncounterArchetype* Archetype, const FVector& NearLocation, FVector& OutSpawnLocation);

	/** Spawn encounter actors */
	bool SpawnEncounterActors(const FActiveEncounter& Encounter);

	/** Cleanup encounter actors */
	void CleanupEncounterActors(const FActiveEncounter& Encounter);

	/** Generate location key for cooldown tracking */
	FString GenerateLocationKey(const FVector& Location) const;

	/** Check performance budget and return if we can continue processing */
	bool CheckPerformanceBudget(float StartTime) const;
};