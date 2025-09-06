#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "EncounterHandleComponent.generated.h"

/**
 * Component that tracks an actor's relationship to the current encounter
 * Provides squad information, role, and state for reliable teardown/recovery
 */
UCLASS(ClassGroup=(Vanguard), meta=(BlueprintSpawnableComponent), BlueprintType, Blueprintable)
class VANGUARD_API UEncounterHandleComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEncounterHandleComponent();

	/** Unique identifier of the encounter this actor belongs to */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Encounter")
	FGuid EncounterId;

	/** Index within the encounter's squad system */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Squad")
	int32 SquadIndex = INDEX_NONE;

	/** Role within the encounter (Leader, Support, Flanker, etc.) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Squad")
	FGameplayTag SquadRole;

	/** Current behavioral state in the encounter */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "State")
	FGameplayTag EncounterState;

	/** Priority level for target selection and resource allocation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Priority")
	int32 Priority = 1;

	/** Whether this actor is essential to the encounter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Encounter")
	bool bIsEssential = false;

	/** Whether this actor can retreat from the encounter */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	bool bCanRetreat = true;

	/** Current aggression level (0.0 = passive, 1.0 = maximum aggression) */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Behavior")
	float AggressionLevel = 0.5f;

	/** Time when this actor joined the encounter */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Timing")
	float EncounterJoinTime = 0.0f;

	/** Tags describing current tactical state */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tactics")
	FGameplayTagContainer TacticalTags;

public:
	/** Initialize with encounter data */
	UFUNCTION(BlueprintCallable, Category = "Encounter")
	void InitializeEncounterHandle(const FGuid& InEncounterId, int32 InSquadIndex, const FGameplayTag& InSquadRole);

	/** Clear encounter association */
	UFUNCTION(BlueprintCallable, Category = "Encounter")
	void ClearEncounterHandle();

	/** Set the encounter state */
	UFUNCTION(BlueprintCallable, Category = "State")
	void SetEncounterState(const FGameplayTag& NewState);

	/** Modify aggression level */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void SetAggressionLevel(float NewLevel);

	/** Add tactical tag */
	UFUNCTION(BlueprintCallable, Category = "Tactics")
	void AddTacticalTag(const FGameplayTag& TacticalTag);

	/** Remove tactical tag */
	UFUNCTION(BlueprintCallable, Category = "Tactics")
	void RemoveTacticalTag(const FGameplayTag& TacticalTag);

	/** Check if has tactical tag */
	UFUNCTION(BlueprintCallable, Category = "Tactics")
	bool HasTacticalTag(const FGameplayTag& TacticalTag) const;

	/** Request retreat from encounter */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	bool RequestRetreat();

	/** Get encounter participation duration */
	UFUNCTION(BlueprintCallable, Category = "Timing")
	float GetEncounterDuration() const;

	/** Check if this actor is currently in an encounter */
	UFUNCTION(BlueprintCallable, Category = "Encounter")
	bool IsInEncounter() const;

	/** Get squad members with the same squad index */
	UFUNCTION(BlueprintCallable, Category = "Squad")
	TArray<AActor*> GetSquadMembers() const;

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	/** Called when encounter state changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "State")
	void OnEncounterStateChanged(const FGameplayTag& OldState, const FGameplayTag& NewState);

	/** Called when aggression level changes */
	UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
	void OnAggressionLevelChanged(float OldLevel, float NewLevel);
};