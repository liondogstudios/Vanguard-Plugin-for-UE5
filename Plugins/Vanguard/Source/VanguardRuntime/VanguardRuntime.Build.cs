using UnrealBuildTool;

public class VanguardRuntime : ModuleRules
{
	public VanguardRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
			}
		);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
			}
		);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"GameplayTags",
				"GameplayAbilities",
				"GameplayTasks",
				"AIModule",
				"NavigationSystem",
				"ToolMenus",
				"Slate",
				"SlateCore",
				"UMG",
				"DeveloperSettings"
			}
		);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Slate",
				"SlateCore",
				"EditorStyle",
				"EditorWidgets",
				"UnrealEd",
				"ToolMenus",
				"PropertyEditor",
				"KismetCompiler",
				"BlueprintGraph",
				"AIGraph",
				"Kismet",
				"KismetWidgets",
				"AssetTools",
				"AssetRegistry",
				"WorldBrowser",
				"SmartObjectsModule"
			}
		);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
		);
	}
}