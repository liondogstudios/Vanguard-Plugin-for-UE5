#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogVanguardEditor, Log, All);

/**
 * Module definition for Vanguard Editor
 */
class FVanguardEditorModule : public IModuleInterface
{
public:
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	/** Register asset type actions */
	void RegisterAssetTypeActions();

	/** Unregister asset type actions */
	void UnregisterAssetTypeActions();

	/** Asset type actions for primary data assets */
	TArray<TSharedPtr<class FAssetTypeActions_Base>> CreatedAssetTypeActions;
};