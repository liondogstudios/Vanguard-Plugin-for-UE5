#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "Data/RivalArchetype.h"
#include "RivalProfileComponent.generated.h"

/**
 * Component that embeds rival identity, history, injuries, and rivalry state on any pawn/NPC
 */
UCLASS(ClassGroup=(Vanguard), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class VANGUARD_API URivalProfileComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URivalProfileComponent();

	/** Unique identifier for this rival */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	FGuid RivalId;

	/** Archetype defining this rival's base characteristics */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Identity")
	TSoftObjectPtr<URivalArchetype> Archetype;

	/** Current traits possessed by this rival */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traits")
	FGameplayTagContainer Traits;

	/** Notoriety level with the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reputation")
	int32 Notoriety = 0;

	/** Kill count against the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "History")
	int32 PlayerKills = 0;

	/** Number of times this rival has been defeated */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "History")
	int32 DeathCount = 0;

	/** Number of times this rival has escaped */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "History")
	int32 EscapeCount = 0;

	/** Injuries and scars accumulated over time */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Injuries")
	FGameplayTagContainer Injuries;

	/** Last known location where this rival was encountered */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Location")
	FVector LastKnownLocation;

	/** Timestamp of last encounter */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Location")
	float LastEncounterTime = 0.0f;

	/** Player tactics this rival has observed */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Adaptation")
	FGameplayTagContainer ObservedPlayerTactics;

	/** Preferred tactics this rival uses */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
	FGameplayTagContainer PreferredTactics;

	/** Current faction affiliation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	FGameplayTag CurrentFaction;

	/** Personal vendetta level against the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reputation")
	float VendettaLevel = 0.0f;

	/** Fear level of this rival towards the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reputation")
	float FearLevel = 0.0f;

	/** Admiration level for the player (can lead to defection) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reputation")
	float AdmirationLevel = 0.0f;

public:
	/** Initialize rival with a unique ID */
	UFUNCTION(BlueprintCallable, Category = "Rival")
	void InitializeRival();

	/** Add a trait to this rival */
	UFUNCTION(BlueprintCallable, Category = "Traits")
	bool AddTrait(const FGameplayTag& TraitTag);

	/** Remove a trait from this rival */
	UFUNCTION(BlueprintCallable, Category = "Traits")
	bool RemoveTrait(const FGameplayTag& TraitTag);

	/** Check if rival has a specific trait */
	UFUNCTION(BlueprintCallable, Category = "Traits")
	bool HasTrait(const FGameplayTag& TraitTag) const;

	/** Record an encounter event */
	UFUNCTION(BlueprintCallable, Category = "History")
	void RecordEncounterEvent(const FGameplayTag& EventType, const FVector& Location);

	/** Add an injury to this rival */
	UFUNCTION(BlueprintCallable, Category = "Injuries")
	void AddInjury(const FGameplayTag& InjuryTag);

	/** Record player tactic observation */
	UFUNCTION(BlueprintCallable, Category = "Adaptation")
	void ObservePlayerTactic(const FGameplayTag& TacticTag);

	/** Modify notoriety level */
	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void ModifyNotoriety(int32 Delta);

	/** Modify vendetta level */
	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void ModifyVendetta(float Delta);

	/** Modify fear level */
	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void ModifyFear(float Delta);

	/** Modify admiration level */
	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void ModifyAdmiration(float Delta);

	/** Get effective rival level based on traits and history */
	UFUNCTION(BlueprintCallable, Category = "Rival")
	int32 GetEffectiveLevel() const;

protected:
	virtual void BeginPlay() override;
};