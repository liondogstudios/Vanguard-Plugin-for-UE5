#include "Subsystems/RivalSystem.h"
#include "VanguardAPI.h"
#include "Components/RivalProfileComponent.h"
#include "Components/FactionAffiliationComponent.h"
#include "Data/FactionArchetype.h"
#include "Save/RivalSaveGame.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

URivalSystem::URivalSystem()
{
}

void URivalSystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogVanguard, Warning, TEXT("Rival System initialized"));
	
	// Initialize faction system
	FactionReputations.Empty();
	FactionHostilities.Empty();
	LoadedFactions.Empty();
	
	// Clear event history
	EventHistory.Empty();
	EventHistory.Reserve(1000); // Reserve space for events
	
	AddEventToHistory(TEXT("Rival System Initialized"));
}

void URivalSystem::Deinitialize()
{
	UE_LOG(LogVanguard, Warning, TEXT("Rival System shutting down"));
	
	// Save state before shutdown
	SaveRivalSystemState();
	
	// Clear all data
	RivalRegistry.Empty();
	FactionReputations.Empty();
	FactionHostilities.Empty();
	LoadedFactions.Empty();
	EventHistory.Empty();
	
	Super::Deinitialize();
}

bool URivalSystem::RegisterRival(AActor* Actor)
{
	if (!Actor)
	{
		UE_LOG(LogVanguard, Warning, TEXT("Attempted to register null actor as rival"));
		return false;
	}
	
	URivalProfileComponent* ProfileComponent = Actor->FindComponentByClass<URivalProfileComponent>();
	if (!ProfileComponent)
	{
		UE_LOG(LogVanguard, Warning, TEXT("Actor %s does not have a RivalProfileComponent"), *Actor->GetName());
		return false;
	}
	
	FGuid RivalId = ProfileComponent->RivalId;
	if (!RivalId.IsValid())
	{
		UE_LOG(LogVanguard, Warning, TEXT("Rival Profile Component has invalid ID"));
		return false;
	}
	
	// Create registry entry
	FRivalRegistryEntry Entry;
	Entry.RivalId = RivalId;
	Entry.LiveActor = Actor;
	Entry.Traits = ProfileComponent->Traits;
	Entry.Notoriety = ProfileComponent->Notoriety;
	Entry.LastKnownLocation = ProfileComponent->LastKnownLocation;
	
	// Get faction from faction component
	if (UFactionAffiliationComponent* FactionComponent = Actor->FindComponentByClass<UFactionAffiliationComponent>())
	{
		Entry.FactionTag = FactionComponent->FactionTag;
	}
	
	// Add to registry
	RivalRegistry.Add(RivalId, Entry);
	
	UE_LOG(LogVanguard, Log, TEXT("Registered rival %s with ID %s"), *Actor->GetName(), *RivalId.ToString());
	AddEventToHistory(FString::Printf(TEXT("Registered rival: %s"), *Actor->GetName()));
	
	return true;
}

void URivalSystem::UnregisterRival(const FGuid& RivalId)
{
	if (RivalRegistry.Contains(RivalId))
	{
		FRivalRegistryEntry* Entry = RivalRegistry.Find(RivalId);
		FString ActorName = Entry && Entry->LiveActor.IsValid() ? Entry->LiveActor->GetName() : TEXT("Unknown");
		
		RivalRegistry.Remove(RivalId);
		UE_LOG(LogVanguard, Log, TEXT("Unregistered rival %s with ID %s"), *ActorName, *RivalId.ToString());
		AddEventToHistory(FString::Printf(TEXT("Unregistered rival: %s"), *ActorName));
	}
}

void URivalSystem::RecordEvent(const FGuid& RivalId, const FGameplayTag& EventTag, const FString& EventData)
{
	if (!RivalId.IsValid() || !EventTag.IsValid())
	{
		return;
	}
	
	FString EventString = FString::Printf(TEXT("Rival %s: %s %s"), 
		*RivalId.ToString(), *EventTag.ToString(), *EventData);
	
	AddEventToHistory(EventString);
	
	// Update rival entry if it exists
	if (FRivalRegistryEntry* Entry = RivalRegistry.Find(RivalId))
	{
		if (Entry->LiveActor.IsValid())
		{
			if (URivalProfileComponent* ProfileComponent = Entry->LiveActor->FindComponentByClass<URivalProfileComponent>())
			{
				UpdateRivalEntry(RivalId, ProfileComponent);
			}
		}
	}
	
	UE_LOG(LogVanguard, Log, TEXT("Recorded event: %s"), *EventString);
}

bool URivalSystem::GetRivalEntry(const FGuid& RivalId, FRivalRegistryEntry& OutEntry) const
{
	if (const FRivalRegistryEntry* Entry = RivalRegistry.Find(RivalId))
	{
		OutEntry = *Entry;
		return true;
	}
	return false;
}

TArray<FGuid> URivalSystem::GetRivalsInFaction(const FGameplayTag& FactionTag) const
{
	TArray<FGuid> Result;
	
	for (const auto& Pair : RivalRegistry)
	{
		if (Pair.Value.FactionTag == FactionTag)
		{
			Result.Add(Pair.Key);
		}
	}
	
	return Result;
}

TArray<FGuid> URivalSystem::GetRivalsWithTrait(const FGameplayTag& TraitTag) const
{
	TArray<FGuid> Result;
	
	for (const auto& Pair : RivalRegistry)
	{
		if (Pair.Value.Traits.HasTagExact(TraitTag))
		{
			Result.Add(Pair.Key);
		}
	}
	
	return Result;
}

TArray<FGuid> URivalSystem::GetRivalsAboveNotoriety(int32 MinNotoriety) const
{
	TArray<FGuid> Result;
	
	for (const auto& Pair : RivalRegistry)
	{
		if (Pair.Value.Notoriety >= MinNotoriety)
		{
			Result.Add(Pair.Key);
		}
	}
	
	return Result;
}

void URivalSystem::ModifyFactionReputation(const FGameplayTag& FactionTag, float Delta)
{
	if (!FactionTag.IsValid())
	{
		return;
	}
	
	float& Reputation = FactionReputations.FindOrAdd(FactionTag, 0.0f);
	Reputation = FMath::Clamp(Reputation + Delta, -100.0f, 100.0f);
	
	UE_LOG(LogVanguard, Log, TEXT("Modified faction %s reputation by %f, new value: %f"), 
		   *FactionTag.ToString(), Delta, Reputation);
	
	AddEventToHistory(FString::Printf(TEXT("Faction %s reputation changed by %f"), *FactionTag.ToString(), Delta));
}

float URivalSystem::GetFactionReputation(const FGameplayTag& FactionTag) const
{
	if (const float* Reputation = FactionReputations.Find(FactionTag))
	{
		return *Reputation;
	}
	return 0.0f; // Neutral by default
}

void URivalSystem::SetFactionHostility(const FGameplayTag& FactionA, const FGameplayTag& FactionB, float HostilityLevel)
{
	if (!FactionA.IsValid() || !FactionB.IsValid())
	{
		return;
	}
	
	HostilityLevel = FMath::Clamp(HostilityLevel, -1.0f, 1.0f);
	
	// Set both directions
	FactionHostilities.FindOrAdd(FactionA).FindOrAdd(FactionB) = HostilityLevel;
	FactionHostilities.FindOrAdd(FactionB).FindOrAdd(FactionA) = HostilityLevel;
	
	UE_LOG(LogVanguard, Log, TEXT("Set hostility between %s and %s to %f"), 
		   *FactionA.ToString(), *FactionB.ToString(), HostilityLevel);
}

float URivalSystem::GetFactionHostility(const FGameplayTag& FactionA, const FGameplayTag& FactionB) const
{
	if (const TMap<FGameplayTag, float>* HostilityMap = FactionHostilities.Find(FactionA))
	{
		if (const float* Hostility = HostilityMap->Find(FactionB))
		{
			return *Hostility;
		}
	}
	return 0.0f; // Neutral by default
}

bool URivalSystem::SaveRivalSystemState(const FString& SlotName)
{
	URivalSaveGame* SaveGame = Cast<URivalSaveGame>(UGameplayStatics::CreateSaveGameObject(URivalSaveGame::StaticClass()));
	if (!SaveGame)
	{
		UE_LOG(LogVanguard, Error, TEXT("Failed to create save game object"));
		return false;
	}
	
	// Set basic save data
	SaveGame->SaveTimestamp = FDateTime::Now();
	SaveGame->SchemaVersion = 1;
	
	// Save rival data
	for (const auto& Pair : RivalRegistry)
	{
		const FRivalRegistryEntry& Entry = Pair.Value;
		FRivalSaveData RivalData;
		
		RivalData.RivalId = Entry.RivalId;
		RivalData.Traits = Entry.Traits;
		RivalData.Notoriety = Entry.Notoriety;
		RivalData.LastKnownLocation = Entry.LastKnownLocation;
		RivalData.CurrentFaction = Entry.FactionTag;
		
		// Get additional data from live actor if available
		if (Entry.LiveActor.IsValid())
		{
			if (URivalProfileComponent* ProfileComp = Entry.LiveActor->FindComponentByClass<URivalProfileComponent>())
			{
				RivalData.PlayerKills = ProfileComp->PlayerKills;
				RivalData.DeathCount = ProfileComp->DeathCount;
				RivalData.EscapeCount = ProfileComp->EscapeCount;
				RivalData.Injuries = ProfileComp->Injuries;
				RivalData.ObservedPlayerTactics = ProfileComp->ObservedPlayerTactics;
				RivalData.PreferredTactics = ProfileComp->PreferredTactics;
				RivalData.VendettaLevel = ProfileComp->VendettaLevel;
				RivalData.FearLevel = ProfileComp->FearLevel;
				RivalData.AdmirationLevel = ProfileComp->AdmirationLevel;
			}
		}
		
		SaveGame->AddRivalData(RivalData);
	}
	
	// Save faction data
	for (const auto& Pair : FactionReputations)
	{
		FFactionSaveData FactionData;
		FactionData.FactionTag = Pair.Key;
		FactionData.PlayerReputation = Pair.Value;
		
		// Add hostility data if available
		if (const TMap<FGameplayTag, float>* HostilityMap = FactionHostilities.Find(Pair.Key))
		{
			FactionData.HostilityMatrix = *HostilityMap;
		}
		
		SaveGame->AddFactionData(FactionData);
	}
	
	// Save recent events
	SaveGame->RecentEvents = GetRecentEvents(100);
	
	// Save to file
	bool bSuccess = UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, 0);
	
	UE_LOG(LogVanguard, Log, TEXT("Saving rival system state to slot %s: %s"), 
		   *SlotName, bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
	
	AddEventToHistory(FString::Printf(TEXT("Saved system state to slot: %s (%s)"), 
		*SlotName, bSuccess ? TEXT("SUCCESS") : TEXT("FAILED")));
	
	return bSuccess;
}

bool URivalSystem::LoadRivalSystemState(const FString& SlotName)
{
	if (!UGameplayStatics::DoesSaveGameExist(SlotName, 0))
	{
		UE_LOG(LogVanguard, Warning, TEXT("Save game slot %s does not exist"), *SlotName);
		return false;
	}
	
	URivalSaveGame* SaveGame = Cast<URivalSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!SaveGame)
	{
		UE_LOG(LogVanguard, Error, TEXT("Failed to load save game from slot %s"), *SlotName);
		return false;
	}
	
	// Validate save data
	if (!SaveGame->ValidateSaveData())
	{
		UE_LOG(LogVanguard, Error, TEXT("Save data validation failed for slot %s"), *SlotName);
		return false;
	}
	
	// Migrate save data if needed
	SaveGame->MigrateSaveData(SaveGame->SchemaVersion);
	
	// Clear existing data
	RivalRegistry.Empty();
	FactionReputations.Empty();
	FactionHostilities.Empty();
	
	// Load rival data
	for (const FRivalSaveData& RivalData : SaveGame->RivalData)
	{
		FRivalRegistryEntry Entry;
		Entry.RivalId = RivalData.RivalId;
		Entry.Traits = RivalData.Traits;
		Entry.Notoriety = RivalData.Notoriety;
		Entry.LastKnownLocation = RivalData.LastKnownLocation;
		Entry.FactionTag = RivalData.CurrentFaction;
		// LiveActor will be null until the rival is spawned again
		
		RivalRegistry.Add(Entry.RivalId, Entry);
	}
	
	// Load faction data
	for (const FFactionSaveData& FactionData : SaveGame->FactionData)
	{
		FactionReputations.Add(FactionData.FactionTag, FactionData.PlayerReputation);
		
		if (FactionData.HostilityMatrix.Num() > 0)
		{
			FactionHostilities.Add(FactionData.FactionTag, FactionData.HostilityMatrix);
		}
	}
	
	// Load event history
	EventHistory.Empty();
	EventHistory.Append(SaveGame->RecentEvents);
	
	UE_LOG(LogVanguard, Log, TEXT("Loaded rival system state from slot %s: %d rivals, %d factions"), 
		   *SlotName, SaveGame->GetRivalCount(), SaveGame->GetFactionCount());
	
	AddEventToHistory(FString::Printf(TEXT("Loaded system state from slot: %s (%d rivals, %d factions)"), 
		*SlotName, SaveGame->GetRivalCount(), SaveGame->GetFactionCount()));
	
	return true;
}

void URivalSystem::ClearAllRivalData()
{
	RivalRegistry.Empty();
	FactionReputations.Empty();
	FactionHostilities.Empty();
	EventHistory.Empty();
	
	UE_LOG(LogVanguard, Warning, TEXT("Cleared all rival system data"));
	AddEventToHistory(TEXT("Cleared all rival data"));
}

int32 URivalSystem::GetRivalCount() const
{
	return RivalRegistry.Num();
}

TArray<FString> URivalSystem::GetRecentEvents(int32 MaxEvents) const
{
	TArray<FString> Result;
	int32 StartIndex = FMath::Max(0, EventHistory.Num() - MaxEvents);
	
	for (int32 i = StartIndex; i < EventHistory.Num(); ++i)
	{
		Result.Add(EventHistory[i]);
	}
	
	return Result;
}

void URivalSystem::UpdateRivalEntry(const FGuid& RivalId, URivalProfileComponent* ProfileComponent)
{
	if (!ProfileComponent)
	{
		return;
	}
	
	if (FRivalRegistryEntry* Entry = RivalRegistry.Find(RivalId))
	{
		Entry->Traits = ProfileComponent->Traits;
		Entry->Notoriety = ProfileComponent->Notoriety;
		Entry->LastKnownLocation = ProfileComponent->LastKnownLocation;
		
		// Update faction if component exists
		if (AActor* Owner = ProfileComponent->GetOwner())
		{
			if (UFactionAffiliationComponent* FactionComponent = Owner->FindComponentByClass<UFactionAffiliationComponent>())
			{
				Entry->FactionTag = FactionComponent->FactionTag;
			}
		}
	}
}

void URivalSystem::AddEventToHistory(const FString& Event)
{
	// Add timestamp
	FDateTime Now = FDateTime::Now();
	FString TimestampedEvent = FString::Printf(TEXT("[%s] %s"), *Now.ToString(), *Event);
	
	EventHistory.Add(TimestampedEvent);
	
	// Keep history size manageable
	if (EventHistory.Num() > 1000)
	{
		EventHistory.RemoveAt(0, 100); // Remove oldest 100 events
	}
}

void URivalSystem::CleanupInvalidRivals()
{
	TArray<FGuid> ToRemove;
	
	for (const auto& Pair : RivalRegistry)
	{
		if (!Pair.Value.LiveActor.IsValid())
		{
			ToRemove.Add(Pair.Key);
		}
	}
	
	for (const FGuid& RivalId : ToRemove)
	{
		RivalRegistry.Remove(RivalId);
	}
	
	if (ToRemove.Num() > 0)
	{
		UE_LOG(LogVanguard, Log, TEXT("Cleaned up %d invalid rival entries"), ToRemove.Num());
	}
}