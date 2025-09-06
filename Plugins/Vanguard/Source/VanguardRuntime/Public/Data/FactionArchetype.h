#pragma once

#include "CoreMinimal.h"
#include "Engine/PrimaryDataAsset.h"
#include "GameplayTagContainer.h"
#include "UObject/SoftObjectPtr.h"
#include "FactionArchetype.generated.h"

/**
 * Defines faction properties, hostility relationships, and territorial behavior
 */
UCLASS(BlueprintType, Blueprintable)
class VANGUARD_API UFactionArchetype : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UFactionArchetype();

	/** Display name of this faction */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FText FactionName;

	/** Faction identification tags */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	FGameplayTagContainer FactionTags;

	/** Team ID for AI perception and hostility systems */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Identity")
	uint8 TeamId = 0;

	/** Faction colors for UI and visual identification */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	FLinearColor PrimaryColor = FLinearColor::White;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	FLinearColor SecondaryColor = FLinearColor::Gray;

	/** Faction emblem or symbol asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Visuals")
	TSoftObjectPtr<UTexture2D> EmblemTexture;

	/** Hostility matrix - which factions this faction is hostile to */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Diplomacy")
	TMap<FGameplayTag, float> HostilityMatrix; // Tag -> Hostility value (-1.0 to 1.0)

	/** Base territories this faction controls */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Territory")
	FGameplayTagContainer BaseTerritories;

	/** Diplomacy rules for relationship changes */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Diplomacy")
	FGameplayTagContainer DiplomacyRules;

	/** Default rival archetypes associated with this faction */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Members")
	TArray<TSoftObjectPtr<class URivalArchetype>> DefaultRivalTypes;

public:
	// UPrimaryDataAsset interface
	virtual FPrimaryAssetId GetPrimaryAssetId() const override;

	/** Get hostility level towards another faction */
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	float GetHostilityTowards(const FGameplayTag& OtherFactionTag) const;

	/** Set hostility level towards another faction */
	UFUNCTION(BlueprintCallable, Category = "Diplomacy")
	void SetHostilityTowards(const FGameplayTag& OtherFactionTag, float HostilityLevel);
};