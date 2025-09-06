#pragma once

#include "CoreMinimal.h"
#include "Engine/PrimaryDataAsset.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "RivalArchetype.generated.h"

class AAIController;

/**
 * Defines a rival character archetype with identity, behavior, and growth rules
 */
UCLASS(BlueprintType, Blueprintable)
class VANGUARD_API URivalArchetype : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	URivalArchetype();

	/** Display name for this rival archetype */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText DisplayName;

	/** Gameplay tags defining this archetype's characteristics */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTagContainer Tags;

	/** AI Controller class to use for this rival type */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "AI")
	TSoftClassPtr<AAIController> AIControllerClass;

	/** Assets defining abilities, loadout, or other character data */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Loadout")
	TArray<TSoftObjectPtr<UObject>> AbilityOrLoadoutAssets;

	/** Base traits that can be assigned to this rival type */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Growth")
	FGameplayTagContainer AllowedTraits;

	/** Voice lines and audio assets for this rival */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Audio")
	TArray<TSoftObjectPtr<USoundBase>> VoiceAssets;

	/** Visual representation assets (mesh, materials, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TArray<TSoftObjectPtr<UObject>> VisualAssets;

	/** Rules for how this rival can grow and evolve */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Growth")
	FGameplayTagContainer GrowthRules;

	/** Spawn templates or blueprints for creating instances */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Spawning")
	TSoftClassPtr<APawn> SpawnTemplate;

public:
	// UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;
};