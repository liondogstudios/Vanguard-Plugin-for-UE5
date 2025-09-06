#include "VanguardAPI.h"

DEFINE_LOG_CATEGORY(LogVanguard);

void FVanguardRuntimeModule::StartupModule()
{
	UE_LOG(LogVanguard, Warning, TEXT("Vanguard Runtime Module Started"));
}

void FVanguardRuntimeModule::ShutdownModule()
{
	UE_LOG(LogVanguard, Warning, TEXT("Vanguard Runtime Module Shutdown"));
}

IMPLEMENT_MODULE(FVanguardRuntimeModule, VanguardRuntime)