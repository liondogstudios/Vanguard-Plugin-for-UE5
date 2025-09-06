#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "GameplayTagContainer.h"
#include "VanguardAIController.generated.h"

class URivalProfileComponent;
class UFactionAffiliationComponent;
class UEncounterHandleComponent;

/**
 * Enhanced AI Controller for Vanguard rivals
 * Integrates with the rivalry system and provides encounter-aware behavior
 */
UCLASS(BlueprintType, Blueprintable)
class VANGUARD_API AVanguardAIController : public AAIController
{
	GENERATED_BODY()

public:
	AVanguardAIController();

protected:
	/** AI Perception component for enhanced sensing */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	class UAIPerceptionComponent* AIPerceptionComponent;

	/** Current tactical state */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Tactics")
	FGameplayTag TacticalState;

	/** Current aggression level (0.0 = passive, 1.0 = maximum aggression) */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Behavior")
	float AggressionLevel = 0.5f;

	/** Current alertness level (0.0 = unaware, 1.0 = fully alert) */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Awareness")
	float AlertnessLevel = 0.0f;

	/** Whether this AI can call for reinforcements */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	bool bCanCallReinforcements = true;

	/** Time since last reinforcement call */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Behavior")
	float TimeSinceLastReinforcementCall = 0.0f;

	/** Minimum time between reinforcement calls */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Behavior")
	float ReinforcementCallCooldown = 30.0f;

	/** Memory of observed player tactics */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Memory")
	FGameplayTagContainer ObservedPlayerTactics;

	/** Preferred tactics for this AI */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tactics")
	FGameplayTagContainer PreferredTactics;

public:
	// AAIController interface
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void OnPossess(APawn* InPawn) override;
	virtual void OnUnPossess() override;

	/** Set tactical state */
	UFUNCTION(BlueprintCallable, Category = "Tactics")
	void SetTacticalState(const FGameplayTag& NewState);

	/** Modify aggression level */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	void ModifyAggression(float Delta, bool bClamp = true);

	/** Modify alertness level */
	UFUNCTION(BlueprintCallable, Category = "Awareness")
	void ModifyAlertness(float Delta, bool bClamp = true);

	/** Observe a player tactic */
	UFUNCTION(BlueprintCallable, Category = "Memory")
	void ObservePlayerTactic(const FGameplayTag& TacticTag);

	/** Check if AI has observed a specific tactic */
	UFUNCTION(BlueprintCallable, Category = "Memory")
	bool HasObservedTactic(const FGameplayTag& TacticTag) const;

	/** Attempt to call for reinforcements */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	bool TryCallReinforcements();

	/** Get current rival profile component */
	UFUNCTION(BlueprintCallable, Category = "Rival")
	URivalProfileComponent* GetRivalProfile() const;

	/** Get current faction affiliation component */
	UFUNCTION(BlueprintCallable, Category = "Faction")
	UFactionAffiliationComponent* GetFactionAffiliation() const;

	/** Get current encounter handle component */
	UFUNCTION(BlueprintCallable, Category = "Encounter")
	UEncounterHandleComponent* GetEncounterHandle() const;

	/** Check if this AI is currently in an encounter */
	UFUNCTION(BlueprintCallable, Category = "Encounter")
	bool IsInEncounter() const;

	/** React to taking damage from the player */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReactToPlayerDamage(float DamageAmount, const FGameplayTag& DamageType);

	/** React to witnessing ally damage */
	UFUNCTION(BlueprintCallable, Category = "Combat")
	void ReactToAllyDamage(AActor* Ally, float DamageAmount);

	/** React to player using a specific tactic */
	UFUNCTION(BlueprintCallable, Category = "Adaptation")
	void ReactToPlayerTactic(const FGameplayTag& TacticTag);

	/** Evaluate and potentially switch tactics */
	UFUNCTION(BlueprintCallable, Category = "Tactics")
	void EvaluateTacticalSwitch();

	/** Get effective aggression based on traits and state */
	UFUNCTION(BlueprintCallable, Category = "Behavior")
	float GetEffectiveAggression() const;

	/** Get effective alertness based on traits and state */
	UFUNCTION(BlueprintCallable, Category = "Awareness")
	float GetEffectiveAlertness() const;

protected:
	/** Initialize AI perception */
	void InitializePerception();

	/** Update tactical behavior based on current state */
	void UpdateTacticalBehavior(float DeltaTime);

	/** Process memory and adaptation */
	void ProcessMemoryAndAdaptation(float DeltaTime);

	/** Handle perception events */
	UFUNCTION()
	void OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors);

	/** Handle target perception events */
	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);

	/** Broadcast tactical state change */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tactics")
	void OnTacticalStateChanged(const FGameplayTag& OldState, const FGameplayTag& NewState);

	/** Broadcast aggression level change */
	UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
	void OnAggressionChanged(float OldLevel, float NewLevel);

	/** Broadcast alertness level change */
	UFUNCTION(BlueprintImplementableEvent, Category = "Awareness")
	void OnAlertnessChanged(float OldLevel, float NewLevel);

	/** Called when reinforcements are successfully requested */
	UFUNCTION(BlueprintImplementableEvent, Category = "Behavior")
	void OnReinforcementsRequested();

	/** Called when a new player tactic is observed */
	UFUNCTION(BlueprintImplementableEvent, Category = "Memory")
	void OnPlayerTacticObserved(const FGameplayTag& TacticTag);

private:
	/** Cached component references */
	UPROPERTY()
	URivalProfileComponent* CachedRivalProfile;

	UPROPERTY()
	UFactionAffiliationComponent* CachedFactionAffiliation;

	UPROPERTY()
	UEncounterHandleComponent* CachedEncounterHandle;
};