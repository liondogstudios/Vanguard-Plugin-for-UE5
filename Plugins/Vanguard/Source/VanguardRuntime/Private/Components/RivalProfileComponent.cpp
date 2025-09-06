#include "Components/RivalProfileComponent.h"
#include "VanguardAPI.h"
#include "Engine/World.h"

URivalProfileComponent::URivalProfileComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	
	// Generate unique ID on construction
	RivalId = FGuid::NewGuid();
	
	// Initialize default values
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

void URivalProfileComponent::BeginPlay()
{
	Super::BeginPlay();
	
	// Register with rival system
	if (UWorld* World = GetWorld())
	{
		// This will be connected to the RivalSystem once it's implemented
		UE_LOG(LogVanguard, Log, TEXT("Rival Profile Component initialized for %s"), 
			   RivalId.IsValid() ? *RivalId.ToString() : TEXT("Invalid ID"));
	}
}

void URivalProfileComponent::InitializeRival()
{
	if (!RivalId.IsValid())
	{
		RivalId = FGuid::NewGuid();
	}
	
	// Reset history
	PlayerKills = 0;
	DeathCount = 0;
	EscapeCount = 0;
	Notoriety = 0;
	VendettaLevel = 0.0f;
	FearLevel = 0.0f;
	AdmirationLevel = 0.0f;
	
	// Clear tags
	Traits.Reset();
	Injuries.Reset();
	ObservedPlayerTactics.Reset();
}

bool URivalProfileComponent::AddTrait(const FGameplayTag& TraitTag)
{
	if (TraitTag.IsValid() && !Traits.HasTagExact(TraitTag))
	{
		Traits.AddTag(TraitTag);
		UE_LOG(LogVanguard, Log, TEXT("Added trait %s to rival %s"), 
			   *TraitTag.ToString(), *RivalId.ToString());
		return true;
	}
	return false;
}

bool URivalProfileComponent::RemoveTrait(const FGameplayTag& TraitTag)
{
	if (TraitTag.IsValid() && Traits.HasTagExact(TraitTag))
	{
		Traits.RemoveTag(TraitTag);
		UE_LOG(LogVanguard, Log, TEXT("Removed trait %s from rival %s"), 
			   *TraitTag.ToString(), *RivalId.ToString());
		return true;
	}
	return false;
}

bool URivalProfileComponent::HasTrait(const FGameplayTag& TraitTag) const
{
	return TraitTag.IsValid() && Traits.HasTagExact(TraitTag);
}

void URivalProfileComponent::RecordEncounterEvent(const FGameplayTag& EventType, const FVector& Location)
{
	LastKnownLocation = Location;
	
	if (UWorld* World = GetWorld())
	{
		LastEncounterTime = World->GetTimeSeconds();
	}
	
	UE_LOG(LogVanguard, Log, TEXT("Recorded encounter event %s for rival %s at location %s"), 
		   *EventType.ToString(), *RivalId.ToString(), *Location.ToString());
}

void URivalProfileComponent::AddInjury(const FGameplayTag& InjuryTag)
{
	if (InjuryTag.IsValid())
	{
		Injuries.AddTag(InjuryTag);
		UE_LOG(LogVanguard, Log, TEXT("Added injury %s to rival %s"), 
			   *InjuryTag.ToString(), *RivalId.ToString());
	}
}

void URivalProfileComponent::ObservePlayerTactic(const FGameplayTag& TacticTag)
{
	if (TacticTag.IsValid())
	{
		ObservedPlayerTactics.AddTag(TacticTag);
		UE_LOG(LogVanguard, Log, TEXT("Rival %s observed player tactic: %s"), 
			   *RivalId.ToString(), *TacticTag.ToString());
	}
}

void URivalProfileComponent::ModifyNotoriety(int32 Delta)
{
	Notoriety = FMath::Max(0, Notoriety + Delta);
	UE_LOG(LogVanguard, Log, TEXT("Modified notoriety for rival %s by %d, new value: %d"), 
		   *RivalId.ToString(), Delta, Notoriety);
}

void URivalProfileComponent::ModifyVendetta(float Delta)
{
	VendettaLevel = FMath::Clamp(VendettaLevel + Delta, 0.0f, 100.0f);
	UE_LOG(LogVanguard, Log, TEXT("Modified vendetta for rival %s by %f, new value: %f"), 
		   *RivalId.ToString(), Delta, VendettaLevel);
}

void URivalProfileComponent::ModifyFear(float Delta)
{
	FearLevel = FMath::Clamp(FearLevel + Delta, 0.0f, 100.0f);
	UE_LOG(LogVanguard, Log, TEXT("Modified fear for rival %s by %f, new value: %f"), 
		   *RivalId.ToString(), Delta, FearLevel);
}

void URivalProfileComponent::ModifyAdmiration(float Delta)
{
	AdmirationLevel = FMath::Clamp(AdmirationLevel + Delta, 0.0f, 100.0f);
	UE_LOG(LogVanguard, Log, TEXT("Modified admiration for rival %s by %f, new value: %f"), 
		   *RivalId.ToString(), Delta, AdmirationLevel);
}

int32 URivalProfileComponent::GetEffectiveLevel() const
{
	// Calculate effective level based on traits, notoriety, and experience
	int32 BaseLevel = 1;
	int32 TraitBonus = Traits.Num();
	int32 NotorietyBonus = Notoriety / 10; // Every 10 notoriety = +1 level
	int32 ExperienceBonus = (PlayerKills + EscapeCount) / 2; // Experience from encounters
	
	return BaseLevel + TraitBonus + NotorietyBonus + ExperienceBonus;
}