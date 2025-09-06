#pragma once

#include "CoreMinimal.h"
#include "Engine/PrimaryDataAsset.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "EncounterArchetype.generated.h"

class UEnvQuery;

/**
 * Defines an encounter type with templates for ambush/convoy/siege/etc.
 */
UCLASS(BlueprintType, Blueprintable)
class VANGUARD_API UEncounterArchetype : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UEncounterArchetype();

	/** Type of encounter (Encounter.Type.Ambush, Encounter.Type.Convoy, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter")
	FGameplayTag EncounterType;

	/** Biome tags where this encounter can spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Encounter")
	FGameplayTagContainer BiomeTags;

	/** Budget cost for this encounter type */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Budget")
	int32 BudgetCost = 1;

	/** EQS asset for finding spawn points */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TSoftObjectPtr<UEnvQuery> EQSAsset;

	/** Squad composition and size rules */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Squad")
	TMap<FGameplayTag, int32> SquadComposition;

	/** AI behavior assets (Behavior Trees, State Trees) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TArray<TSoftObjectPtr<UObject>> AIBehaviorAssets;

	/** Objectives and win/fail conditions */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objectives")
	FGameplayTagContainer ObjectiveTags;

	/** Fallback encounter types if this one fails */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Failover")
	TArray<TSoftObjectPtr<UEncounterArchetype>> FallbackEncounters;

	/** Cooldown time after this encounter completes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Timing")
	float CooldownDuration = 300.0f; // 5 minutes

	/** Conditions that must be met for this encounter to spawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Conditions")
	FGameplayTagContainer RequiredConditions;

public:
	// UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};