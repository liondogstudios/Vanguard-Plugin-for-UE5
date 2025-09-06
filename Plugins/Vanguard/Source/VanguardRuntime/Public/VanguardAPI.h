#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystems/WorldSubsystem.h"
#include "Subsystems/EngineSubsystem.h"
#include "Components/ActorComponent.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "Engine/PrimaryDataAsset.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "GenericTeamAgentInterface.h"
#include "AIController.h"
#include "UObject/SoftObjectPtr.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVanguard, Log, All);

/**
 * Module definition for Vanguard Runtime
 */
class FVanguardRuntimeModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

// Module export macros
#if PLATFORM_WINDOWS
	#ifdef VANGUARDRUNTIME_EXPORTS
		#define VANGUARD_API __declspec(dllexport)
	#else
		#define VANGUARD_API __declspec(dllimport)
	#endif
#else
	#define VANGUARD_API
#endif