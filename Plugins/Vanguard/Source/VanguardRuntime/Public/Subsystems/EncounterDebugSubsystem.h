#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"
#include "GameplayTagContainer.h"
#include "EncounterDebugSubsystem.generated.h"

class UCanvas;
class UFont;

/** Structure to hold debug visualization data */
USTRUCT(BlueprintType)
struct VANGUARD_API FVanguardDebugData
{
	GENERATED_BODY()

	/** Whether debug visualization is enabled */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowDebugInfo = false;

	/** Whether to show rival profiles */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowRivalProfiles = false;

	/** Whether to show encounter areas */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowEncounterAreas = false;

	/** Whether to show faction relationships */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowFactionRelationships = false;

	/** Whether to show performance metrics */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowPerformanceMetrics = false;

	/** Whether to show cooldown areas */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShowCooldownAreas = false;

	/** Debug text scale */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DebugTextScale = 1.0f;

	FVanguardDebugData()
	{
		bShowDebugInfo = false;
		bShowRivalProfiles = false;
		bShowEncounterAreas = false;
		bShowFactionRelationships = false;
		bShowPerformanceMetrics = false;
		bShowCooldownAreas = false;
		DebugTextScale = 1.0f;
	}
};

/** Performance tracking data */
USTRUCT(BlueprintType)
struct VANGUARD_API FVanguardPerformanceData
{
	GENERATED_BODY()

	/** Active rival count */
	UPROPERTY(BlueprintReadOnly)
	int32 ActiveRivalCount = 0;

	/** Active encounter count */
	UPROPERTY(BlueprintReadOnly)
	int32 ActiveEncounterCount = 0;

	/** Budget usage percentage */
	UPROPERTY(BlueprintReadOnly)
	float BudgetUsagePercentage = 0.0f;

	/** Frame time spent on Vanguard systems (milliseconds) */
	UPROPERTY(BlueprintReadOnly)
	float VanguardFrameTimeMs = 0.0f;

	/** Memory usage (estimated, in MB) */
	UPROPERTY(BlueprintReadOnly)
	float MemoryUsageMB = 0.0f;

	/** Recent events count */
	UPROPERTY(BlueprintReadOnly)
	int32 RecentEventsCount = 0;

	FVanguardPerformanceData()
	{
		ActiveRivalCount = 0;
		ActiveEncounterCount = 0;
		BudgetUsagePercentage = 0.0f;
		VanguardFrameTimeMs = 0.0f;
		MemoryUsageMB = 0.0f;
		RecentEventsCount = 0;
	}
};

/**
 * Engine subsystem for debugging and monitoring Vanguard systems
 * Provides overlays, heatmaps, performance counters, and encounter replay
 */
UCLASS(BlueprintType)
class VANGUARD_API UEncounterDebugSubsystem : public UEngineSubsystem
{
	GENERATED_BODY()

public:
	UEncounterDebugSubsystem();

	// USubsystem interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

protected:
	/** Current debug settings */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	FVanguardDebugData DebugSettings;

	/** Performance tracking data */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Performance")
	FVanguardPerformanceData PerformanceData;

	/** Whether debug drawing is currently active */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	bool bIsDebugDrawingActive = false;

	/** Debug console commands */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	TArray<FString> RegisteredCommands;

public:
	/** Enable/disable debug visualization */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetDebugVisualization(bool bEnabled);

	/** Toggle specific debug feature */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void ToggleDebugFeature(const FString& FeatureName);

	/** Draw debug information on HUD */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawDebugHUD(UCanvas* Canvas, const class APlayerController* PC);

	/** Update performance metrics */
	UFUNCTION(BlueprintCallable, Category = "Performance")
	void UpdatePerformanceMetrics();

	/** Get current performance data */
	UFUNCTION(BlueprintCallable, Category = "Performance")
	FVanguardPerformanceData GetPerformanceData() const { return PerformanceData; }

	/** Log encounter event for debugging */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void LogEncounterEvent(const FString& EventType, const FVector& Location, const FString& Details);

	/** Draw encounter debug visualization */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawEncounterDebug(UWorld* World);

	/** Draw rival debug visualization */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawRivalDebug(UWorld* World);

	/** Draw faction debug visualization */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawFactionDebug(UWorld* World);

	/** Draw performance debug visualization */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void DrawPerformanceDebug(UCanvas* Canvas);

	/** Execute debug command */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	bool ExecuteDebugCommand(const FString& Command, const TArray<FString>& Parameters);

	/** Get debug settings */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	FVanguardDebugData GetDebugSettings() const { return DebugSettings; }

	/** Set debug settings */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void SetDebugSettings(const FVanguardDebugData& NewSettings);

	/** Create debug heatmap for encounters */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	void CreateEncounterHeatmap(UWorld* World, float GridSize = 1000.0f);

	/** Export debug data to file */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	bool ExportDebugData(const FString& FilePath);

	/** Import debug data from file */
	UFUNCTION(BlueprintCallable, Category = "Debug")
	bool ImportDebugData(const FString& FilePath);

protected:
	/** Register console commands */
	void RegisterConsoleCommands();

	/** Unregister console commands */
	void UnregisterConsoleCommands();

	/** Draw debug text with proper formatting */
	void DrawDebugText(UCanvas* Canvas, const FString& Text, float X, float Y, const FLinearColor& Color = FLinearColor::White) const;

	/** Draw debug line in world */
	void DrawDebugLine(UWorld* World, const FVector& Start, const FVector& End, const FLinearColor& Color, float Duration = 0.0f) const;

	/** Draw debug sphere in world */
	void DrawDebugSphere(UWorld* World, const FVector& Location, float Radius, const FLinearColor& Color, float Duration = 0.0f) const;

	/** Get font for debug rendering */
	UFont* GetDebugFont() const;

	/** Calculate memory usage estimation */
	float CalculateMemoryUsage() const;

	/** Format performance metrics for display */
	TArray<FString> FormatPerformanceMetrics() const;

	/** Console command handlers */
	void HandleVanguardDebugCommand(const TArray<FString>& Args);
	void HandleVanguardStatsCommand(const TArray<FString>& Args);
	void HandleVanguardClearCommand(const TArray<FString>& Args);
	void HandleVanguardDumpCommand(const TArray<FString>& Args);

	/** Debug event history */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Debug")
	TArray<FString> DebugEventHistory;

	/** Maximum debug events to keep */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	int32 MaxDebugEvents = 1000;

private:
	/** Console command objects */
	TArray<class IConsoleObject*> ConsoleCommands;
};