#include "AI/VanguardAIController.h"
#include "VanguardAPI.h"
#include "Components/RivalProfileComponent.h"
#include "Components/FactionAffiliationComponent.h"
#include "Components/EncounterHandleComponent.h"
#include "Subsystems/RivalSystem.h"
#include "Subsystems/EncounterDirectorSubsystem.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Damage.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Pawn.h"

AVanguardAIController::AVanguardAIController()
{
	// Enable ticking
	PrimaryActorTick.bCanEverTick = true;
	
	// Create AI perception component
	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerceptionComponent"));
	SetPerceptionComponent(*AIPerceptionComponent);
	
	// Initialize default values
	AggressionLevel = 0.5f;
	AlertnessLevel = 0.0f;
	bCanCallReinforcements = true;
	TimeSinceLastReinforcementCall = 0.0f;
	ReinforcementCallCooldown = 30.0f;
	
	// Initialize cached references
	CachedRivalProfile = nullptr;
	CachedFactionAffiliation = nullptr;
	CachedEncounterHandle = nullptr;
}

void AVanguardAIController::BeginPlay()
{
	Super::BeginPlay();
	
	// Initialize AI perception
	InitializePerception();
	
	UE_LOG(LogVanguard, Log, TEXT("Vanguard AI Controller initialized"));
}

void AVanguardAIController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	// Update cooldown timers
	TimeSinceLastReinforcementCall += DeltaTime;
	
	// Update tactical behavior
	UpdateTacticalBehavior(DeltaTime);
	
	// Process memory and adaptation
	ProcessMemoryAndAdaptation(DeltaTime);
}

void AVanguardAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	
	if (InPawn)
	{
		// Cache component references
		CachedRivalProfile = InPawn->FindComponentByClass<URivalProfileComponent>();
		CachedFactionAffiliation = InPawn->FindComponentByClass<UFactionAffiliationComponent>();
		CachedEncounterHandle = InPawn->FindComponentByClass<UEncounterHandleComponent>();
		
		// Register with rival system if we have a rival profile
		if (CachedRivalProfile)
		{
			if (UGameInstance* GameInstance = GetGameInstance())
			{
				if (URivalSystem* RivalSystem = GameInstance->GetSubsystem<URivalSystem>())
				{
					RivalSystem->RegisterRival(InPawn);
				}
			}
		}
		
		// Set initial tactical state
		SetTacticalState(FGameplayTag::RequestGameplayTag(TEXT("AI.Tactic.Patrol")));
		
		UE_LOG(LogVanguard, Log, TEXT("Vanguard AI Controller possessed pawn: %s"), *InPawn->GetName());
	}
}

void AVanguardAIController::OnUnPossess()
{
	// Unregister from rival system
	if (CachedRivalProfile && GetPawn())
	{
		if (UGameInstance* GameInstance = GetGameInstance())
		{
			if (URivalSystem* RivalSystem = GameInstance->GetSubsystem<URivalSystem>())
			{
				RivalSystem->UnregisterRival(CachedRivalProfile->RivalId);
			}
		}
	}
	
	// Clear cached references
	CachedRivalProfile = nullptr;
	CachedFactionAffiliation = nullptr;
	CachedEncounterHandle = nullptr;
	
	Super::OnUnPossess();
}

void AVanguardAIController::SetTacticalState(const FGameplayTag& NewState)
{
	if (NewState != TacticalState)
	{
		FGameplayTag OldState = TacticalState;
		TacticalState = NewState;
		
		// Update blackboard if available
		if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
		{
			BlackboardComp->SetValueAsString(TEXT("TacticalState"), TacticalState.ToString());
		}
		
		OnTacticalStateChanged(OldState, NewState);
		
		UE_LOG(LogVanguard, Log, TEXT("AI %s tactical state changed: %s -> %s"), 
			   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"),
			   *OldState.ToString(), *NewState.ToString());
	}
}

void AVanguardAIController::ModifyAggression(float Delta, bool bClamp)
{
	float OldLevel = AggressionLevel;
	AggressionLevel += Delta;
	
	if (bClamp)
	{
		AggressionLevel = FMath::Clamp(AggressionLevel, 0.0f, 1.0f);
	}
	
	if (!FMath::IsNearlyEqual(OldLevel, AggressionLevel))
	{
		// Update blackboard
		if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
		{
			BlackboardComp->SetValueAsFloat(TEXT("AggressionLevel"), AggressionLevel);
		}
		
		OnAggressionChanged(OldLevel, AggressionLevel);
		
		UE_LOG(LogVanguard, Log, TEXT("AI %s aggression changed: %f -> %f"), 
			   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), OldLevel, AggressionLevel);
	}
}

void AVanguardAIController::ModifyAlertness(float Delta, bool bClamp)
{
	float OldLevel = AlertnessLevel;
	AlertnessLevel += Delta;
	
	if (bClamp)
	{
		AlertnessLevel = FMath::Clamp(AlertnessLevel, 0.0f, 1.0f);
	}
	
	if (!FMath::IsNearlyEqual(OldLevel, AlertnessLevel))
	{
		// Update blackboard
		if (UBlackboardComponent* BlackboardComp = GetBlackboardComponent())
		{
			BlackboardComp->SetValueAsFloat(TEXT("AlertnessLevel"), AlertnessLevel);
		}
		
		OnAlertnessChanged(OldLevel, AlertnessLevel);
		
		UE_LOG(LogVanguard, Log, TEXT("AI %s alertness changed: %f -> %f"), 
			   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), OldLevel, AlertnessLevel);
	}
}

void AVanguardAIController::ObservePlayerTactic(const FGameplayTag& TacticTag)
{
	if (TacticTag.IsValid() && !ObservedPlayerTactics.HasTagExact(TacticTag))
	{
		ObservedPlayerTactics.AddTag(TacticTag);
		
		// Update rival profile if available
		if (CachedRivalProfile)
		{
			CachedRivalProfile->ObservePlayerTactic(TacticTag);
		}
		
		OnPlayerTacticObserved(TacticTag);
		
		UE_LOG(LogVanguard, Log, TEXT("AI %s observed player tactic: %s"), 
			   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), *TacticTag.ToString());
	}
}

bool AVanguardAIController::HasObservedTactic(const FGameplayTag& TacticTag) const
{
	return TacticTag.IsValid() && ObservedPlayerTactics.HasTagExact(TacticTag);
}

bool AVanguardAIController::TryCallReinforcements()
{
	if (!bCanCallReinforcements || TimeSinceLastReinforcementCall < ReinforcementCallCooldown)
	{
		return false;
	}
	
	// Check if in encounter and can call reinforcements
	if (!IsInEncounter())
	{
		return false;
	}
	
	// Reset cooldown
	TimeSinceLastReinforcementCall = 0.0f;
	
	// This would trigger reinforcement spawning logic
	OnReinforcementsRequested();
	
	UE_LOG(LogVanguard, Log, TEXT("AI %s called for reinforcements"), 
		   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"));
	
	return true;
}

URivalProfileComponent* AVanguardAIController::GetRivalProfile() const
{
	return CachedRivalProfile;
}

UFactionAffiliationComponent* AVanguardAIController::GetFactionAffiliation() const
{
	return CachedFactionAffiliation;
}

UEncounterHandleComponent* AVanguardAIController::GetEncounterHandle() const
{
	return CachedEncounterHandle;
}

bool AVanguardAIController::IsInEncounter() const
{
	return CachedEncounterHandle && CachedEncounterHandle->IsInEncounter();
}

void AVanguardAIController::ReactToPlayerDamage(float DamageAmount, const FGameplayTag& DamageType)
{
	// Increase aggression based on damage taken
	float AggressionIncrease = FMath::Clamp(DamageAmount / 100.0f, 0.05f, 0.3f);
	ModifyAggression(AggressionIncrease);
	
	// Increase alertness
	ModifyAlertness(0.2f);
	
	// Observe damage type as player tactic
	if (DamageType.IsValid())
	{
		ObservePlayerTactic(DamageType);
	}
	
	// Consider calling reinforcements if heavily damaged
	if (DamageAmount > 50.0f && AggressionLevel > 0.7f)
	{
		TryCallReinforcements();
	}
	
	UE_LOG(LogVanguard, Log, TEXT("AI %s reacted to player damage: %f (%s)"), 
		   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), DamageAmount, *DamageType.ToString());
}

void AVanguardAIController::ReactToAllyDamage(AActor* Ally, float DamageAmount)
{
	if (!Ally)
	{
		return;
	}
	
	// Increase aggression when allies are damaged
	float AggressionIncrease = FMath::Clamp(DamageAmount / 200.0f, 0.02f, 0.15f);
	ModifyAggression(AggressionIncrease);
	
	// Increase alertness
	ModifyAlertness(0.1f);
	
	UE_LOG(LogVanguard, Log, TEXT("AI %s reacted to ally damage on %s: %f"), 
		   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), *Ally->GetName(), DamageAmount);
}

void AVanguardAIController::ReactToPlayerTactic(const FGameplayTag& TacticTag)
{
	// Observe the tactic
	ObservePlayerTactic(TacticTag);
	
	// Consider tactical switch based on observed tactic
	EvaluateTacticalSwitch();
	
	UE_LOG(LogVanguard, Log, TEXT("AI %s reacted to player tactic: %s"), 
		   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), *TacticTag.ToString());
}

void AVanguardAIController::EvaluateTacticalSwitch()
{
	// This would contain logic to switch tactics based on observed player behavior
	// For now, just log that we're evaluating
	UE_LOG(LogVanguard, VerboseLog, TEXT("AI %s evaluating tactical switch"), 
		   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"));
}

float AVanguardAIController::GetEffectiveAggression() const
{
	float EffectiveAggression = AggressionLevel;
	
	// Modify based on rival traits
	if (CachedRivalProfile)
	{
		if (CachedRivalProfile->HasTrait(FGameplayTag::RequestGameplayTag(TEXT("Trait.Aggressive"))))
		{
			EffectiveAggression += 0.2f;
		}
		if (CachedRivalProfile->HasTrait(FGameplayTag::RequestGameplayTag(TEXT("Trait.Cowardly"))))
		{
			EffectiveAggression -= 0.3f;
		}
	}
	
	return FMath::Clamp(EffectiveAggression, 0.0f, 1.0f);
}

float AVanguardAIController::GetEffectiveAlertness() const
{
	float EffectiveAlertness = AlertnessLevel;
	
	// Modify based on rival traits
	if (CachedRivalProfile)
	{
		if (CachedRivalProfile->HasTrait(FGameplayTag::RequestGameplayTag(TEXT("Trait.Vigilant"))))
		{
			EffectiveAlertness += 0.3f;
		}
		if (CachedRivalProfile->HasTrait(FGameplayTag::RequestGameplayTag(TEXT("Trait.Oblivious"))))
		{
			EffectiveAlertness -= 0.2f;
		}
	}
	
	return FMath::Clamp(EffectiveAlertness, 0.0f, 1.0f);
}

void AVanguardAIController::InitializePerception()
{
	if (!AIPerceptionComponent)
	{
		return;
	}
	
	// Configure sight sense
	UAISenseConfig_Sight* SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));
	if (SightConfig)
	{
		SightConfig->SightRadius = 1500.0f;
		SightConfig->LoseSightRadius = 1600.0f;
		SightConfig->PeripheralVisionAngleDegrees = 90.0f;
		SightConfig->SetMaxAge(5.0f);
		SightConfig->AutoSuccessRangeFromLastSeenLocation = 520.0f;
		SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
		SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
		SightConfig->DetectionByAffiliation.bDetectEnemies = true;
		
		AIPerceptionComponent->SetDominantSense(*SightConfig->GetSenseImplementation());
		AIPerceptionComponent->ConfigureSense(*SightConfig);
	}
	
	// Configure hearing sense
	UAISenseConfig_Hearing* HearingConfig = CreateDefaultSubobject<UAISenseConfig_Hearing>(TEXT("HearingConfig"));
	if (HearingConfig)
	{
		HearingConfig->HearingRange = 1200.0f;
		HearingConfig->SetMaxAge(3.0f);
		HearingConfig->DetectionByAffiliation.bDetectNeutrals = true;
		HearingConfig->DetectionByAffiliation.bDetectFriendlies = true;
		HearingConfig->DetectionByAffiliation.bDetectEnemies = true;
		
		AIPerceptionComponent->ConfigureSense(*HearingConfig);
	}
	
	// Configure damage sense
	UAISenseConfig_Damage* DamageConfig = CreateDefaultSubobject<UAISenseConfig_Damage>(TEXT("DamageConfig"));
	if (DamageConfig)
	{
		DamageConfig->SetMaxAge(10.0f);
		
		AIPerceptionComponent->ConfigureSense(*DamageConfig);
	}
	
	// Bind perception events
	AIPerceptionComponent->OnPerceptionUpdated.AddDynamic(this, &AVanguardAIController::OnPerceptionUpdated);
	AIPerceptionComponent->OnTargetPerceptionUpdated.AddDynamic(this, &AVanguardAIController::OnTargetPerceptionUpdated);
}

void AVanguardAIController::UpdateTacticalBehavior(float DeltaTime)
{
	// This would contain logic to update AI behavior based on current state
	// For now, just track that we're updating
}

void AVanguardAIController::ProcessMemoryAndAdaptation(float DeltaTime)
{
	// This would contain logic to process memories and adapt behavior
	// For now, just track that we're processing
}

void AVanguardAIController::OnPerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	// Handle general perception updates
	for (AActor* Actor : UpdatedActors)
	{
		if (Actor && Actor->IsA<APawn>())
		{
			// Check if this is the player
			if (Actor->GetClass()->GetName().Contains(TEXT("Player")) || 
				Actor->GetClass()->GetName().Contains(TEXT("Character")))
			{
				// Increase alertness when detecting player
				ModifyAlertness(0.1f);
			}
		}
	}
}

void AVanguardAIController::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor)
	{
		return;
	}
	
	// React to specific stimuli
	if (Stimulus.WasSuccessfullySensed())
	{
		// Increase alertness when successfully sensing targets
		ModifyAlertness(0.05f);
		
		// Log perception event
		UE_LOG(LogVanguard, VerboseLog, TEXT("AI %s perceived %s"), 
			   GetPawn() ? *GetPawn()->GetName() : TEXT("Unknown"), *Actor->GetName());
	}
}