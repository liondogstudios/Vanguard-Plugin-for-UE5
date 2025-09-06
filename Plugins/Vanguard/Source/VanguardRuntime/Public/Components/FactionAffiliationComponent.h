#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GenericTeamAgentInterface.h"
#include "GameplayTagContainer.h"
#include "FactionAffiliationComponent.generated.h"

/**
 * Component that provides team affiliation for AI perception and hostility logic
 * Integrates with UE's IGenericTeamAgentInterface
 */
UCLASS(ClassGroup=(Vanguard), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class VANGUARD_API UFactionAffiliationComponent : public UActorComponent, public IGenericTeamAgentInterface
{
	GENERATED_BODY()

public:
	UFactionAffiliationComponent();

	/** Team ID for AI perception system (0 = Neutral by convention) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	uint8 TeamId = 0;

	/** Faction tag this actor belongs to */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	FGameplayTag FactionTag;

	/** Additional faction-related tags */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	FGameplayTagContainer FactionTags;

	/** Current reputation with the player */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Reputation")
	float PlayerReputation = 0.0f;

	/** Whether this actor can change factions dynamically */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction")
	bool bCanDefect = false;

	/** Threshold for faction switching based on player reputation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Faction", meta = (EditCondition = "bCanDefect"))
	float DefectionThreshold = 75.0f;

public:
	// IGenericTeamAgentInterface implementation
	virtual FGenericTeamId GetGenericTeamId() const override;
	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;

	/** Set the team ID and update related systems */
	UFUNCTION(BlueprintCallable, Category = "Faction")
	void SetTeamId(uint8 NewTeamId);

	/** Set the faction tag and update team ID if needed */
	UFUNCTION(BlueprintCallable, Category = "Faction")
	void SetFactionTag(const FGameplayTag& NewFactionTag);

	/** Modify reputation with the player */
	UFUNCTION(BlueprintCallable, Category = "Reputation")
	void ModifyPlayerReputation(float Delta);

	/** Check if this actor is hostile to another team */
	UFUNCTION(BlueprintCallable, Category = "Faction")
	bool IsHostileToTeam(uint8 OtherTeamId) const;

	/** Check if this actor is hostile to another faction */
	UFUNCTION(BlueprintCallable, Category = "Faction")
	bool IsHostileToFaction(const FGameplayTag& OtherFactionTag) const;

	/** Attempt to defect to a new faction */
	UFUNCTION(BlueprintCallable, Category = "Faction")
	bool TryDefectToFaction(const FGameplayTag& NewFactionTag);

	/** Get the faction archetype data asset */
	UFUNCTION(BlueprintCallable, Category = "Faction")
	class UFactionArchetype* GetFactionArchetype() const;

protected:
	virtual void BeginPlay() override;

	/** Update team attitude when faction changes */
	void UpdateTeamAttitude();

	/** Broadcast faction change to interested systems */
	UFUNCTION(BlueprintImplementableEvent, Category = "Faction")
	void OnFactionChanged(const FGameplayTag& OldFaction, const FGameplayTag& NewFaction);
};