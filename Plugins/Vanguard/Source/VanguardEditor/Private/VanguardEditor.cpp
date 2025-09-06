#include "VanguardEditor.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"

DEFINE_LOG_CATEGORY(LogVanguardEditor);

void FVanguardEditorModule::StartupModule()
{
	UE_LOG(LogVanguardEditor, Warning, TEXT("Vanguard Editor Module Started"));
	
	// Register asset type actions for primary data assets
	RegisterAssetTypeActions();
}

void FVanguardEditorModule::ShutdownModule()
{
	UE_LOG(LogVanguardEditor, Warning, TEXT("Vanguard Editor Module Shutdown"));
	
	// Unregister asset type actions
	UnregisterAssetTypeActions();
}

void FVanguardEditorModule::RegisterAssetTypeActions()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	
	// Register asset type actions for our primary data assets
	// This would include actions for RivalArchetype, EncounterArchetype, etc.
	// For now, just log that we're registering them
	UE_LOG(LogVanguardEditor, Log, TEXT("Registering Vanguard asset type actions"));
}

void FVanguardEditorModule::UnregisterAssetTypeActions()
{
	// Unregister all asset type actions
	if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
	{
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		
		for (auto& Action : CreatedAssetTypeActions)
		{
			AssetTools.UnregisterAssetTypeActions(Action.ToSharedRef());
		}
	}
	
	CreatedAssetTypeActions.Empty();
}

IMPLEMENT_MODULE(FVanguardEditorModule, VanguardEditor)