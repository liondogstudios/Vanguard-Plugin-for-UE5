#include "Subsystems/EncounterDebugSubsystem.h"
#include "VanguardAPI.h"
#include "Subsystems/RivalSystem.h"
#include "Subsystems/EncounterDirectorSubsystem.h"
#include "Components/RivalProfileComponent.h"
#include "Components/EncounterHandleComponent.h"
#include "Engine/World.h"
#include "Engine/Canvas.h"
#include "Engine/Font.h"
#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "DrawDebugHelpers.h"
#include "HAL/FileManager.h"

UEncounterDebugSubsystem::UEncounterDebugSubsystem()
{
	MaxDebugEvents = 1000;
	bIsDebugDrawingActive = false;
}

void UEncounterDebugSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	UE_LOG(LogVanguard, Warning, TEXT("Encounter Debug Subsystem initialized"));
	
	// Initialize debug settings
	DebugSettings = FVanguardDebugData();
	PerformanceData = FVanguardPerformanceData();
	
	// Clear event history
	DebugEventHistory.Empty();
	DebugEventHistory.Reserve(MaxDebugEvents);
	
	// Register console commands
	RegisterConsoleCommands();
}

void UEncounterDebugSubsystem::Deinitialize()
{
	UE_LOG(LogVanguard, Warning, TEXT("Encounter Debug Subsystem shutting down"));
	
	// Unregister console commands
	UnregisterConsoleCommands();
	
	// Clear debug data
	DebugEventHistory.Empty();
	
	Super::Deinitialize();
}

void UEncounterDebugSubsystem::SetDebugVisualization(bool bEnabled)
{
	DebugSettings.bShowDebugInfo = bEnabled;
	bIsDebugDrawingActive = bEnabled;
	
	UE_LOG(LogVanguard, Log, TEXT("Debug visualization %s"), bEnabled ? TEXT("enabled") : TEXT("disabled"));
}

void UEncounterDebugSubsystem::ToggleDebugFeature(const FString& FeatureName)
{
	if (FeatureName == TEXT("RivalProfiles"))
	{
		DebugSettings.bShowRivalProfiles = !DebugSettings.bShowRivalProfiles;
		UE_LOG(LogVanguard, Log, TEXT("Rival profiles debug: %s"), 
			   DebugSettings.bShowRivalProfiles ? TEXT("ON") : TEXT("OFF"));
	}
	else if (FeatureName == TEXT("EncounterAreas"))
	{
		DebugSettings.bShowEncounterAreas = !DebugSettings.bShowEncounterAreas;
		UE_LOG(LogVanguard, Log, TEXT("Encounter areas debug: %s"), 
			   DebugSettings.bShowEncounterAreas ? TEXT("ON") : TEXT("OFF"));
	}
	else if (FeatureName == TEXT("FactionRelationships"))
	{
		DebugSettings.bShowFactionRelationships = !DebugSettings.bShowFactionRelationships;
		UE_LOG(LogVanguard, Log, TEXT("Faction relationships debug: %s"), 
			   DebugSettings.bShowFactionRelationships ? TEXT("ON") : TEXT("OFF"));
	}
	else if (FeatureName == TEXT("Performance"))
	{
		DebugSettings.bShowPerformanceMetrics = !DebugSettings.bShowPerformanceMetrics;
		UE_LOG(LogVanguard, Log, TEXT("Performance metrics debug: %s"), 
			   DebugSettings.bShowPerformanceMetrics ? TEXT("ON") : TEXT("OFF"));
	}
	else if (FeatureName == TEXT("Cooldowns"))
	{
		DebugSettings.bShowCooldownAreas = !DebugSettings.bShowCooldownAreas;
		UE_LOG(LogVanguard, Log, TEXT("Cooldown areas debug: %s"), 
			   DebugSettings.bShowCooldownAreas ? TEXT("ON") : TEXT("OFF"));
	}
}

void UEncounterDebugSubsystem::DrawDebugHUD(UCanvas* Canvas, const APlayerController* PC)
{
	if (!Canvas || !DebugSettings.bShowDebugInfo)
	{
		return;
	}
	
	// Update performance metrics
	UpdatePerformanceMetrics();
	
	// Draw performance metrics if enabled
	if (DebugSettings.bShowPerformanceMetrics)
	{
		DrawPerformanceDebug(Canvas);
	}
	
	// Draw debug information for the current world
	if (UWorld* World = PC ? PC->GetWorld() : nullptr)
	{
		if (DebugSettings.bShowEncounterAreas)
		{
			DrawEncounterDebug(World);
		}
		
		if (DebugSettings.bShowRivalProfiles)
		{
			DrawRivalDebug(World);
		}
		
		if (DebugSettings.bShowFactionRelationships)
		{
			DrawFactionDebug(World);
		}
	}
}

void UEncounterDebugSubsystem::UpdatePerformanceMetrics()
{
	// Get rival system data
	if (UGameInstance* GameInstance = UGameInstance::GetGameInstance(GetWorld()))
	{
		if (URivalSystem* RivalSystem = GameInstance->GetSubsystem<URivalSystem>())
		{
			PerformanceData.ActiveRivalCount = RivalSystem->GetRivalCount();
			PerformanceData.RecentEventsCount = RivalSystem->GetRecentEvents(50).Num();
		}
	}
	
	// Get encounter director data
	if (UWorld* World = GetWorld())
	{
		if (UEncounterDirectorSubsystem* EncounterDirector = World->GetSubsystem<UEncounterDirectorSubsystem>())
		{
			PerformanceData.ActiveEncounterCount = EncounterDirector->GetActiveEncounterCount();
			
			int32 UsedBudget = EncounterDirector->GetCurrentBudgetUsage();
			int32 MaxBudget = UsedBudget + EncounterDirector->GetAvailableBudget();
			PerformanceData.BudgetUsagePercentage = MaxBudget > 0 ? (float)UsedBudget / MaxBudget * 100.0f : 0.0f;
		}
	}
	
	// Calculate memory usage
	PerformanceData.MemoryUsageMB = CalculateMemoryUsage();
	
	// Frame time would be measured by a performance profiler
	PerformanceData.VanguardFrameTimeMs = 0.5f; // Placeholder
}

void UEncounterDebugSubsystem::LogEncounterEvent(const FString& EventType, const FVector& Location, const FString& Details)
{
	FDateTime Now = FDateTime::Now();
	FString EventString = FString::Printf(TEXT("[%s] %s at %s: %s"), 
		*Now.ToString(), *EventType, *Location.ToString(), *Details);
	
	DebugEventHistory.Add(EventString);
	
	// Trim history if too long
	if (DebugEventHistory.Num() > MaxDebugEvents)
	{
		DebugEventHistory.RemoveAt(0);
	}
	
	UE_LOG(LogVanguard, Log, TEXT("Debug Event: %s"), *EventString);
}

void UEncounterDebugSubsystem::DrawEncounterDebug(UWorld* World)
{
	if (!World)
	{
		return;
	}
	
	UEncounterDirectorSubsystem* EncounterDirector = World->GetSubsystem<UEncounterDirectorSubsystem>();
	if (!EncounterDirector)
	{
		return;
	}
	
	// Draw active encounters
	TArray<FActiveEncounter> ActiveEncounters = EncounterDirector->GetAllActiveEncounters();
	for (const FActiveEncounter& Encounter : ActiveEncounters)
	{
		// Draw encounter area
		DrawDebugSphere(World, Encounter.Location, 500.0f, FLinearColor::Red, 0.0f);
		
		// Draw encounter info
		FString EncounterInfo = FString::Printf(TEXT("Encounter\nBudget: %d\nParticipants: %d"), 
			Encounter.BudgetCost, Encounter.Participants.Num());
		
		DrawDebugString(World, Encounter.Location + FVector(0, 0, 100), EncounterInfo, 
			nullptr, FColor::White, 0.0f);
	}
}

void UEncounterDebugSubsystem::DrawRivalDebug(UWorld* World)
{
	if (!World)
	{
		return;
	}
	
	// Find all actors with rival profile components
	for (TActorIterator<AActor> ActorItr(World); ActorItr; ++ActorItr)
	{
		AActor* Actor = *ActorItr;
		if (URivalProfileComponent* RivalComponent = Actor->FindComponentByClass<URivalProfileComponent>())
		{
			FVector ActorLocation = Actor->GetActorLocation();
			
			// Draw rival info
			FString RivalInfo = FString::Printf(TEXT("Rival\nNotoriety: %d\nTraits: %d"), 
				RivalComponent->Notoriety, RivalComponent->Traits.Num());
			
			DrawDebugString(World, ActorLocation + FVector(0, 0, 150), RivalInfo, 
				nullptr, FColor::Yellow, 0.0f);
			
			// Draw rival sphere
			DrawDebugSphere(World, ActorLocation, 100.0f, FLinearColor::Yellow, 0.0f);
		}
	}
}

void UEncounterDebugSubsystem::DrawFactionDebug(UWorld* World)
{
	if (!World)
	{
		return;
	}
	
	// This would draw faction relationship lines and territory boundaries
	// For now, just log that we're drawing faction debug
	UE_LOG(LogVanguard, VerboseLog, TEXT("Drawing faction debug visualization"));
}

void UEncounterDebugSubsystem::DrawPerformanceDebug(UCanvas* Canvas)
{
	if (!Canvas)
	{
		return;
	}
	
	TArray<FString> Metrics = FormatPerformanceMetrics();
	
	float Y = 100.0f;
	DrawDebugText(Canvas, TEXT("=== VANGUARD PERFORMANCE ==="), 50.0f, Y, FLinearColor::Cyan);
	Y += 25.0f;
	
	for (const FString& Metric : Metrics)
	{
		DrawDebugText(Canvas, Metric, 50.0f, Y, FLinearColor::White);
		Y += 20.0f;
	}
}

bool UEncounterDebugSubsystem::ExecuteDebugCommand(const FString& Command, const TArray<FString>& Parameters)
{
	if (Command == TEXT("toggle"))
	{
		if (Parameters.Num() > 0)
		{
			ToggleDebugFeature(Parameters[0]);
			return true;
		}
	}
	else if (Command == TEXT("clear"))
	{
		DebugEventHistory.Empty();
		UE_LOG(LogVanguard, Log, TEXT("Cleared debug event history"));
		return true;
	}
	else if (Command == TEXT("dump"))
	{
		for (const FString& Event : DebugEventHistory)
		{
			UE_LOG(LogVanguard, Log, TEXT("%s"), *Event);
		}
		return true;
	}
	
	return false;
}

void UEncounterDebugSubsystem::SetDebugSettings(const FVanguardDebugData& NewSettings)
{
	DebugSettings = NewSettings;
	bIsDebugDrawingActive = DebugSettings.bShowDebugInfo;
}

void UEncounterDebugSubsystem::CreateEncounterHeatmap(UWorld* World, float GridSize)
{
	if (!World)
	{
		return;
	}
	
	UE_LOG(LogVanguard, Log, TEXT("Creating encounter heatmap with grid size %f"), GridSize);
	
	// This would create a visual heatmap of encounter frequency
	// Implementation would depend on specific visualization requirements
}

bool UEncounterDebugSubsystem::ExportDebugData(const FString& FilePath)
{
	FString DebugDataString;
	
	// Add performance data
	DebugDataString += TEXT("=== VANGUARD DEBUG DATA EXPORT ===\n");
	DebugDataString += FString::Printf(TEXT("Export Time: %s\n"), *FDateTime::Now().ToString());
	DebugDataString += FString::Printf(TEXT("Active Rivals: %d\n"), PerformanceData.ActiveRivalCount);
	DebugDataString += FString::Printf(TEXT("Active Encounters: %d\n"), PerformanceData.ActiveEncounterCount);
	DebugDataString += FString::Printf(TEXT("Budget Usage: %.1f%%\n"), PerformanceData.BudgetUsagePercentage);
	DebugDataString += TEXT("\n=== EVENT HISTORY ===\n");
	
	// Add event history
	for (const FString& Event : DebugEventHistory)
	{
		DebugDataString += Event + TEXT("\n");
	}
	
	// Write to file
	bool bSuccess = FFileHelper::SaveStringToFile(DebugDataString, *FilePath);
	
	UE_LOG(LogVanguard, Log, TEXT("Export debug data to %s: %s"), 
		   *FilePath, bSuccess ? TEXT("SUCCESS") : TEXT("FAILED"));
	
	return bSuccess;
}

bool UEncounterDebugSubsystem::ImportDebugData(const FString& FilePath)
{
	FString ImportedData;
	if (FFileHelper::LoadFileToString(ImportedData, *FilePath))
	{
		UE_LOG(LogVanguard, Log, TEXT("Imported debug data from %s"), *FilePath);
		// Process imported data as needed
		return true;
	}
	
	UE_LOG(LogVanguard, Warning, TEXT("Failed to import debug data from %s"), *FilePath);
	return false;
}

void UEncounterDebugSubsystem::RegisterConsoleCommands()
{
	// Register console commands for debugging
	IConsoleManager& ConsoleManager = IConsoleManager::Get();
	
	ConsoleManager.RegisterConsoleCommand(
		TEXT("vanguard.debug"),
		TEXT("Toggle Vanguard debug visualization"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEncounterDebugSubsystem::HandleVanguardDebugCommand),
		ECVF_Cheat
	);
	
	ConsoleManager.RegisterConsoleCommand(
		TEXT("vanguard.stats"),
		TEXT("Show Vanguard performance statistics"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEncounterDebugSubsystem::HandleVanguardStatsCommand),
		ECVF_Cheat
	);
	
	ConsoleManager.RegisterConsoleCommand(
		TEXT("vanguard.clear"),
		TEXT("Clear Vanguard debug data"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEncounterDebugSubsystem::HandleVanguardClearCommand),
		ECVF_Cheat
	);
	
	ConsoleManager.RegisterConsoleCommand(
		TEXT("vanguard.dump"),
		TEXT("Dump Vanguard debug events to log"),
		FConsoleCommandWithArgsDelegate::CreateUObject(this, &UEncounterDebugSubsystem::HandleVanguardDumpCommand),
		ECVF_Cheat
	);
	
	RegisteredCommands.Add(TEXT("vanguard.debug"));
	RegisteredCommands.Add(TEXT("vanguard.stats"));
	RegisteredCommands.Add(TEXT("vanguard.clear"));
	RegisteredCommands.Add(TEXT("vanguard.dump"));
}

void UEncounterDebugSubsystem::UnregisterConsoleCommands()
{
	IConsoleManager& ConsoleManager = IConsoleManager::Get();
	
	for (const FString& Command : RegisteredCommands)
	{
		ConsoleManager.UnregisterConsoleObject(*Command);
	}
	
	RegisteredCommands.Empty();
}

void UEncounterDebugSubsystem::DrawDebugText(UCanvas* Canvas, const FString& Text, float X, float Y, const FLinearColor& Color) const
{
	if (!Canvas)
	{
		return;
	}
	
	UFont* Font = GetDebugFont();
	if (Font)
	{
		Canvas->SetDrawColor(FColor(Color.ToFColor(true)));
		Canvas->DrawText(Font, Text, X, Y, DebugSettings.DebugTextScale, DebugSettings.DebugTextScale);
	}
}

void UEncounterDebugSubsystem::DrawDebugLine(UWorld* World, const FVector& Start, const FVector& End, const FLinearColor& Color, float Duration) const
{
	if (World)
	{
		DrawDebugLine(World, Start, End, Color.ToFColor(true), false, Duration);
	}
}

void UEncounterDebugSubsystem::DrawDebugSphere(UWorld* World, const FVector& Location, float Radius, const FLinearColor& Color, float Duration) const
{
	if (World)
	{
		::DrawDebugSphere(World, Location, Radius, 12, Color.ToFColor(true), false, Duration);
	}
}

UFont* UEncounterDebugSubsystem::GetDebugFont() const
{
	// Use engine default font
	return GEngine ? GEngine->GetMediumFont() : nullptr;
}

float UEncounterDebugSubsystem::CalculateMemoryUsage() const
{
	// Rough estimation of memory usage
	float EstimatedMB = 0.0f;
	
	// Count memory from various sources
	EstimatedMB += DebugEventHistory.Num() * 0.001f; // Rough estimate per event
	EstimatedMB += PerformanceData.ActiveRivalCount * 0.01f; // Rough estimate per rival
	EstimatedMB += PerformanceData.ActiveEncounterCount * 0.005f; // Rough estimate per encounter
	
	return EstimatedMB;
}

TArray<FString> UEncounterDebugSubsystem::FormatPerformanceMetrics() const
{
	TArray<FString> Metrics;
	
	Metrics.Add(FString::Printf(TEXT("Active Rivals: %d"), PerformanceData.ActiveRivalCount));
	Metrics.Add(FString::Printf(TEXT("Active Encounters: %d"), PerformanceData.ActiveEncounterCount));
	Metrics.Add(FString::Printf(TEXT("Budget Usage: %.1f%%"), PerformanceData.BudgetUsagePercentage));
	Metrics.Add(FString::Printf(TEXT("Frame Time: %.2f ms"), PerformanceData.VanguardFrameTimeMs));
	Metrics.Add(FString::Printf(TEXT("Memory Usage: %.2f MB"), PerformanceData.MemoryUsageMB));
	Metrics.Add(FString::Printf(TEXT("Recent Events: %d"), PerformanceData.RecentEventsCount));
	
	return Metrics;
}

void UEncounterDebugSubsystem::HandleVanguardDebugCommand(const TArray<FString>& Args)
{
	if (Args.Num() == 0)
	{
		SetDebugVisualization(!DebugSettings.bShowDebugInfo);
	}
	else
	{
		ToggleDebugFeature(Args[0]);
	}
}

void UEncounterDebugSubsystem::HandleVanguardStatsCommand(const TArray<FString>& Args)
{
	UpdatePerformanceMetrics();
	
	UE_LOG(LogVanguard, Warning, TEXT("=== VANGUARD PERFORMANCE STATS ==="));
	for (const FString& Metric : FormatPerformanceMetrics())
	{
		UE_LOG(LogVanguard, Warning, TEXT("%s"), *Metric);
	}
}

void UEncounterDebugSubsystem::HandleVanguardClearCommand(const TArray<FString>& Args)
{
	DebugEventHistory.Empty();
	UE_LOG(LogVanguard, Log, TEXT("Cleared Vanguard debug data"));
}

void UEncounterDebugSubsystem::HandleVanguardDumpCommand(const TArray<FString>& Args)
{
	UE_LOG(LogVanguard, Warning, TEXT("=== VANGUARD DEBUG EVENT HISTORY ==="));
	for (const FString& Event : DebugEventHistory)
	{
		UE_LOG(LogVanguard, Warning, TEXT("%s"), *Event);
	}
}