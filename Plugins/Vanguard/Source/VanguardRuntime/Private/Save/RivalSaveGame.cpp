#include "Save/RivalSaveGame.h"
#include "VanguardAPI.h"

URivalSaveGame::URivalSaveGame()
{
	SchemaVersion = 1;
	SaveTimestamp = FDateTime::Now();
	TotalPlaytime = 0.0f;
}

void URivalSaveGame::AddRivalData(const FRivalSaveData& RivalSaveData)
{
	// Remove existing data for this rival
	RemoveRivalData(RivalSaveData.RivalId);
	
	// Add new data
	RivalData.Add(RivalSaveData);
	
	UE_LOG(LogVanguard, Log, TEXT("Added rival save data for ID: %s"), *RivalSaveData.RivalId.ToString());
}

void URivalSaveGame::RemoveRivalData(const FGuid& RivalId)
{
	int32 RemovedCount = RivalData.RemoveAll([RivalId](const FRivalSaveData& Data)
	{
		return Data.RivalId == RivalId;
	});
	
	if (RemovedCount > 0)
	{
		UE_LOG(LogVanguard, Log, TEXT("Removed rival save data for ID: %s"), *RivalId.ToString());
	}
}

bool URivalSaveGame::GetRivalData(const FGuid& RivalId, FRivalSaveData& OutRivalData) const
{
	for (const FRivalSaveData& Data : RivalData)
	{
		if (Data.RivalId == RivalId)
		{
			OutRivalData = Data;
			return true;
		}
	}
	return false;
}

void URivalSaveGame::AddFactionData(const FFactionSaveData& FactionSaveData)
{
	// Remove existing data for this faction
	int32 RemovedCount = FactionData.RemoveAll([&FactionSaveData](const FFactionSaveData& Data)
	{
		return Data.FactionTag == FactionSaveData.FactionTag;
	});
	
	// Add new data
	FactionData.Add(FactionSaveData);
	
	UE_LOG(LogVanguard, Log, TEXT("Added faction save data for: %s"), *FactionSaveData.FactionTag.ToString());
}

bool URivalSaveGame::GetFactionData(const FGameplayTag& FactionTag, FFactionSaveData& OutFactionData) const
{
	for (const FFactionSaveData& Data : FactionData)
	{
		if (Data.FactionTag == FactionTag)
		{
			OutFactionData = Data;
			return true;
		}
	}
	return false;
}

void URivalSaveGame::SetLocationCooldown(const FString& LocationId, float CooldownEndTime)
{
	LocationCooldowns.Add(LocationId, CooldownEndTime);
	UE_LOG(LogVanguard, VerboseLog, TEXT("Set cooldown for location %s until %f"), *LocationId, CooldownEndTime);
}

bool URivalSaveGame::IsLocationOnCooldown(const FString& LocationId, float CurrentTime) const
{
	if (const float* CooldownEndTime = LocationCooldowns.Find(LocationId))
	{
		return CurrentTime < *CooldownEndTime;
	}
	return false; // No cooldown if not found
}

void URivalSaveGame::AddEventToHistory(const FString& Event)
{
	// Add timestamp
	FDateTime Now = FDateTime::Now();
	FString TimestampedEvent = FString::Printf(TEXT("[%s] %s"), *Now.ToString(), *Event);
	
	RecentEvents.Add(TimestampedEvent);
	
	// Clean up periodically
	CleanupEventHistory();
}

void URivalSaveGame::UpdatePlayerStat(const FGameplayTag& StatTag, int32 Value)
{
	if (StatTag.IsValid())
	{
		PlayerStats.Add(StatTag, Value);
		UE_LOG(LogVanguard, VerboseLog, TEXT("Updated player stat %s to %d"), *StatTag.ToString(), Value);
	}
}

int32 URivalSaveGame::GetPlayerStat(const FGameplayTag& StatTag) const
{
	if (const int32* Stat = PlayerStats.Find(StatTag))
	{
		return *Stat;
	}
	return 0; // Default value
}

void URivalSaveGame::SetWorldStateFlag(const FGameplayTag& Flag, bool bEnabled)
{
	if (Flag.IsValid())
	{
		if (bEnabled)
		{
			WorldStateFlags.AddTag(Flag);
		}
		else
		{
			WorldStateFlags.RemoveTag(Flag);
		}
		
		UE_LOG(LogVanguard, VerboseLog, TEXT("Set world state flag %s to %s"), 
			   *Flag.ToString(), bEnabled ? TEXT("true") : TEXT("false"));
	}
}

bool URivalSaveGame::HasWorldStateFlag(const FGameplayTag& Flag) const
{
	return Flag.IsValid() && WorldStateFlags.HasTagExact(Flag);
}

bool URivalSaveGame::ValidateSaveData() const
{
	// Basic validation checks
	
	// Check schema version
	if (SchemaVersion <= 0)
	{
		UE_LOG(LogVanguard, Error, TEXT("Invalid schema version: %d"), SchemaVersion);
		return false;
	}
	
	// Check for duplicate rival IDs
	TSet<FGuid> UniqueRivalIds;
	for (const FRivalSaveData& Data : RivalData)
	{
		if (!Data.RivalId.IsValid())
		{
			UE_LOG(LogVanguard, Error, TEXT("Found rival with invalid ID"));
			return false;
		}
		
		if (UniqueRivalIds.Contains(Data.RivalId))
		{
			UE_LOG(LogVanguard, Error, TEXT("Found duplicate rival ID: %s"), *Data.RivalId.ToString());
			return false;
		}
		
		UniqueRivalIds.Add(Data.RivalId);
	}
	
	// Check for duplicate faction tags
	TSet<FGameplayTag> UniqueFactionTags;
	for (const FFactionSaveData& Data : FactionData)
	{
		if (!Data.FactionTag.IsValid())
		{
			UE_LOG(LogVanguard, Error, TEXT("Found faction with invalid tag"));
			return false;
		}
		
		if (UniqueFactionTags.Contains(Data.FactionTag))
		{
			UE_LOG(LogVanguard, Error, TEXT("Found duplicate faction tag: %s"), *Data.FactionTag.ToString());
			return false;
		}
		
		UniqueFactionTags.Add(Data.FactionTag);
	}
	
	UE_LOG(LogVanguard, Log, TEXT("Save data validation passed: %d rivals, %d factions"), 
		   RivalData.Num(), FactionData.Num());
	
	return true;
}

bool URivalSaveGame::MigrateSaveData(int32 FromVersion)
{
	if (FromVersion >= SchemaVersion)
	{
		// No migration needed
		return true;
	}
	
	UE_LOG(LogVanguard, Log, TEXT("Migrating save data from version %d to %d"), FromVersion, SchemaVersion);
	
	// Future migration logic would go here
	// For now, just update the schema version
	// SchemaVersion = CURRENT_SCHEMA_VERSION;
	
	return true;
}

void URivalSaveGame::CleanupEventHistory()
{
	// Keep only the most recent events to prevent save file bloat
	const int32 MaxEvents = 500;
	
	if (RecentEvents.Num() > MaxEvents)
	{
		int32 EventsToRemove = RecentEvents.Num() - MaxEvents;
		RecentEvents.RemoveAt(0, EventsToRemove);
		
		UE_LOG(LogVanguard, VerboseLog, TEXT("Cleaned up %d old events from save data"), EventsToRemove);
	}
}