#pragma once

#include "CoreMinimal.h"
#include "Engine/PrimaryDataAsset.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "TraitArchetype.generated.h"

/**
 * Defines traits that can be assigned to rivals (Pyromaniac, Cowardly Sniper, Beast Tamer, etc.)
 */
UCLASS(BlueprintType, Blueprintable)
class VANGUARD_API UTraitArchetype : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UTraitArchetype();

	/** Display name of this trait */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText TraitName;

	/** Description of what this trait does */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText Description;

	/** Gameplay tags that define this trait */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTagContainer TraitTags;

	/** Visual modifiers applied when this trait is active */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TArray<TSoftObjectPtr<UObject>> VisualModifiers;

	/** Gameplay effects or abilities granted by this trait */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gameplay")
	TArray<TSoftObjectPtr<UObject>> GameplayModifiers;

	/** Attribute modifiers (damage bonuses, resistances, etc.) */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Attributes")
	TMap<FGameplayTag, float> AttributeModifiers;

	/** Prerequisites - other traits that must be present */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Requirements")
	FGameplayTagContainer PrerequisiteTraits;

	/** Incompatible traits that cannot coexist with this one */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Requirements")
	FGameplayTagContainer IncompatibleTraits;

	/** Conditions under which this trait can be acquired */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Acquisition")
	FGameplayTagContainer AcquisitionConditions;

	/** Weight/probability for random acquisition */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Acquisition")
	float AcquisitionWeight = 1.0f;

	/** Maximum number of this trait that can be applied */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stacking")
	int32 MaxStacks = 1;

	/** Whether this trait persists through death/respawn */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Persistence")
	bool bPersistsThroughDeath = true;

public:
	// UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Check if this trait is compatible with a set of existing traits */
	UFUNCTION(BlueprintCallable, Category = "Traits")
	bool IsCompatibleWith(const FGameplayTagContainer& ExistingTraits) const;

	/** Check if prerequisites are met for this trait */
	UFUNCTION(BlueprintCallable, Category = "Traits")
	bool ArePrerequisitesMet(const FGameplayTagContainer& ExistingTraits) const;
};