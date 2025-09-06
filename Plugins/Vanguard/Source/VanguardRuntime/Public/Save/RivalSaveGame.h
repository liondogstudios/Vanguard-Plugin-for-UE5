#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "RivalSaveGame.generated.h"

/** Save data for individual rivals */
USTRUCT(BlueprintType)
struct VANGUARD_API FRivalSaveData
{
	GENERATED_BODY()

	/** Unique identifier */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FGuid RivalId;

	/** Rival archetype asset path */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FString ArchetypePath;

	/** Current traits */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FGameplayTagContainer Traits;

	/** Notoriety level */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	int32 Notoriety = 0;

	/** Kill count against player */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	int32 PlayerKills = 0;

	/** Death count */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	int32 DeathCount = 0;

	/** Escape count */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	int32 EscapeCount = 0;

	/** Injuries and scars */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FGameplayTagContainer Injuries;

	/** Last known location */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FVector LastKnownLocation;

	/** Last encounter timestamp */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	float LastEncounterTime = 0.0f;

	/** Observed player tactics */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FGameplayTagContainer ObservedPlayerTactics;

	/** Preferred tactics */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FGameplayTagContainer PreferredTactics;

	/** Current faction */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FGameplayTag CurrentFaction;

	/** Vendetta level */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	float VendettaLevel = 0.0f;

	/** Fear level */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	float FearLevel = 0.0f;

	/** Admiration level */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	float AdmirationLevel = 0.0f;

	/** Cell or world partition location identifier */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FString LastKnownCell;

	/** Pending retaliations or encounters */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	TArray<FString> PendingRetaliations;

	FRivalSaveData()
	{
		RivalId = FGuid::NewGuid();
		Notoriety = 0;
		PlayerKills = 0;
		DeathCount = 0;
		EscapeCount = 0;
		LastKnownLocation = FVector::ZeroVector;
		LastEncounterTime = 0.0f;
		VendettaLevel = 0.0f;
		FearLevel = 0.0f;
		AdmirationLevel = 0.0f;
	}
};

/** Save data for faction relationships */
USTRUCT(BlueprintType)
struct VANGUARD_API FFactionSaveData
{
	GENERATED_BODY()

	/** Faction tag */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FGameplayTag FactionTag;

	/** Player reputation with this faction */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	float PlayerReputation = 0.0f;

	/** Hostility relationships with other factions */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	TMap<FGameplayTag, float> HostilityMatrix;

	/** Controlled territories */
	UPROPERTY(SaveGame, BlueprintReadOnly)
	FGameplayTagContainer ControlledTerritories;

	FFactionSaveData()
	{
		PlayerReputation = 0.0f;
	}
};

/**
 * Save game class for the Vanguard Rival System
 * Stores rival roster, relationships, faction data, and encounter history
 */
UCLASS(BlueprintType)
class VANGUARD_API URivalSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	URivalSaveGame();

	/** Schema version for save file migration */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "System")
	int32 SchemaVersion = 1;

	/** Timestamp when this save was created */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "System")
	FDateTime SaveTimestamp;

	/** Total playtime when this save was created */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "System")
	float TotalPlaytime = 0.0f;

	/** All rival data */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Rivals")
	TArray<FRivalSaveData> RivalData;

	/** All faction data */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Factions")
	TArray<FFactionSaveData> FactionData;

	/** Global encounter cooldowns per location */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Encounters")
	TMap<FString, float> LocationCooldowns;

	/** Pending encounters that were scheduled but not executed */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Encounters")
	TArray<FString> PendingEncounters;

	/** Recent event history for debugging */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "History")
	TArray<FString> RecentEvents;

	/** Player statistics */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "Stats")
	TMap<FGameplayTag, int32> PlayerStats;

	/** World state flags */
	UPROPERTY(SaveGame, BlueprintReadOnly, Category = "World")
	FGameplayTagContainer WorldStateFlags;

public:
	/** Add or update rival data */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	void AddRivalData(const FRivalSaveData& RivalSaveData);

	/** Remove rival data */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	void RemoveRivalData(const FGuid& RivalId);

	/** Get rival data by ID */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	bool GetRivalData(const FGuid& RivalId, FRivalSaveData& OutRivalData) const;

	/** Add or update faction data */
	UFUNCTION(BlueprintCallable, Category = "Factions")
	void AddFactionData(const FFactionSaveData& FactionSaveData);

	/** Get faction data by tag */
	UFUNCTION(BlueprintCallable, Category = "Factions")
	bool GetFactionData(const FGameplayTag& FactionTag, FFactionSaveData& OutFactionData) const;

	/** Set location cooldown */
	UFUNCTION(BlueprintCallable, Category = "Encounters")
	void SetLocationCooldown(const FString& LocationId, float CooldownEndTime);

	/** Check if location is on cooldown */
	UFUNCTION(BlueprintCallable, Category = "Encounters")
	bool IsLocationOnCooldown(const FString& LocationId, float CurrentTime) const;

	/** Add event to history */
	UFUNCTION(BlueprintCallable, Category = "History")
	void AddEventToHistory(const FString& Event);

	/** Update player stat */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	void UpdatePlayerStat(const FGameplayTag& StatTag, int32 Value);

	/** Get player stat */
	UFUNCTION(BlueprintCallable, Category = "Stats")
	int32 GetPlayerStat(const FGameplayTag& StatTag) const;

	/** Set world state flag */
	UFUNCTION(BlueprintCallable, Category = "World")
	void SetWorldStateFlag(const FGameplayTag& Flag, bool bEnabled);

	/** Check world state flag */
	UFUNCTION(BlueprintCallable, Category = "World")
	bool HasWorldStateFlag(const FGameplayTag& Flag) const;

	/** Validate save data integrity */
	UFUNCTION(BlueprintCallable, Category = "System")
	bool ValidateSaveData() const;

	/** Migrate save data from older schema versions */
	UFUNCTION(BlueprintCallable, Category = "System")
	bool MigrateSaveData(int32 FromVersion);

	/** Get number of rivals saved */
	UFUNCTION(BlueprintCallable, Category = "Rivals")
	int32 GetRivalCount() const { return RivalData.Num(); }

	/** Get number of factions saved */
	UFUNCTION(BlueprintCallable, Category = "Factions")
	int32 GetFactionCount() const { return FactionData.Num(); }

protected:
	/** Clean up old events to prevent save file bloat */
	void CleanupEventHistory();
};