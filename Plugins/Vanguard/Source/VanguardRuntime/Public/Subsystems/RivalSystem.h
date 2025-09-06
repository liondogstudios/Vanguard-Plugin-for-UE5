#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameplayTagContainer.h"
#include "Components/RivalProfileComponent.h"
#include "RivalSystem.generated.h"

class URivalSaveGame;
class UFactionArchetype;

/** Structure to hold rival registry data */
USTRUCT(BlueprintType)
struct VANGUARD_API FRivalRegistryEntry
{
	GENERATED_BODY()

	/** Unique identifier */
	UPROPERTY(BlueprintReadOnly)
	FGuid RivalId;

	/** Weak reference to the live actor */
	UPROPERTY(BlueprintReadOnly)
	TWeakObjectPtr<AActor> LiveActor;

	/** Current traits */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTagContainer Traits;

	/** Notoriety level */
	UPROPERTY(BlueprintReadOnly)
	int32 Notoriety = 0;

	/** Last known location */
	UPROPERTY(BlueprintReadOnly)
	FVector LastKnownLocation;

	/** Faction affiliation */
	UPROPERTY(BlueprintReadOnly)
	FGameplayTag FactionTag;

	FRivalRegistryEntry()
	{
		RivalId = FGuid::NewGuid();
		Notoriety = 0;
		LastKnownLocation = FVector::ZeroVector;
	}
};

/**
 * Global registry of rivals, factions, relationships, and reputations
 * Game-wide state that outlives worlds
 */
UCLASS(BlueprintType)
class VANGUARD_API URivalSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	URivalSystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	/** Registry of all rivals in the system */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rivals")
	TMap<FGuid, FRivalRegistryEntry> RivalRegistry;

	/** Global faction relationships */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factions")
	TMap<FGameplayTag, float> FactionReputations; // FactionTag -> Player reputation

	/** Active faction hostilities */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factions")
	TMap<FGameplayTag, TMap<FGameplayTag, float>> FactionHostilities; // Faction1 -> Faction2 -> Hostility

	/** Loaded faction archetypes */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Factions")
	TMap<FGameplayTag, TSoftObjectPtr<UFactionArchetype>> LoadedFactions;

	/** Event history for analytics and debugging */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "History")
	TArray<FString> EventHistory;

public:
	/** Register a rival actor with the system */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	bool RegisterRival(AActor* Actor);

	/** Unregister a rival actor */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	void UnregisterRival(const FGuid& RivalId);

	/** Record an event for a specific rival */
	UFUNCTION(BlueprintCallable, Category = "Events")
	void RecordEvent(const FGuid& RivalId, const FGameplayTag& EventTag, const FString& EventData = TEXT(""));

	/** Get rival registry entry by ID */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	bool GetRivalEntry(const FGuid& RivalId, FRivalRegistryEntry& OutEntry) const;

	/** Get all rivals in a faction */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	TArray<FGuid> GetRivalsInFaction(const FGameplayTag& FactionTag) const;

	/** Get all rivals with a specific trait */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	TArray<FGuid> GetRivalsWithTrait(const FGameplayTag& TraitTag) const;

	/** Get all rivals above a notoriety threshold */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	TArray<FGuid> GetRivalsAboveNotoriety(int32 MinNotoriety) const;

	/** Modify faction reputation with player */
	UFUNCTION(BlueprintCallable, Category = "Factions")
	void ModifyFactionReputation(const FGameplayTag& FactionTag, float Delta);

	/** Get faction reputation with player */
	UFUNCTION(BlueprintCallable, Category = "Factions")
	float GetFactionReputation(const FGameplayTag& FactionTag) const;

	/** Set hostility between two factions */
	UFUNCTION(BlueprintCallable, Category = "Factions")
	void SetFactionHostility(const FGameplayTag& FactionA, const FGameplayTag& FactionB, float HostilityLevel);

	/** Get hostility level between two factions */
	UFUNCTION(BlueprintCallable, Category = "Factions")
	float GetFactionHostility(const FGameplayTag& FactionA, const FGameplayTag& FactionB) const;

	/** Save rival system state */
	UFUNCTION(BlueprintCallable, Category = "Persistence")
	bool SaveRivalSystemState(const FString& SlotName = TEXT("RivalSystem"));

	/** Load rival system state */
	UFUNCTION(BlueprintCallable, Category = "Persistence")
	bool LoadRivalSystemState(const FString& SlotName = TEXT("RivalSystem"));

	/** Clear all rival data (for new game) */
	UFUNCTION(BlueprintCallable, Category = "System")
	void ClearAllRivalData();

	/** Get number of registered rivals */
	UFUNCTION(BlueprintCallable, Category = "System")
	int32 GetRivalCount() const;

	/** Get recent event history */
	UFUNCTION(BlueprintCallable, Category = "System")
	TArray<FString> GetRecentEvents(int32 MaxEvents = 50) const;

protected:
	/** Update a rival's registry entry from their component */
	void UpdateRivalEntry(const FGuid& RivalId, URivalProfileComponent* ProfileComponent);

	/** Add event to history */
	void AddEventToHistory(const FString& Event);

	/** Cleanup invalid rival entries */
	void CleanupInvalidRivals();
};