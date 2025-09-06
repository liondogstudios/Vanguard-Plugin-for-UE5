using UnrealBuildTool;

public class VanguardEditor : ModuleRules
{
	public VanguardEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"VanguardRuntime",
				"EditorSubsystem",
				"UnrealEd",
				"ToolMenus",
				"EditorWidgets",
				"EditorStyle",
				"Slate",
				"SlateCore",
				"PropertyEditor",
				"DetailCustomizations",
				"ComponentVisualizers"
			}
		);
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"GameplayTags",
				"GameplayTagsEditor",
				"AssetTools",
				"AssetRegistry",
				"KismetCompiler",
				"BlueprintGraph",
				"KismetWidgets",
				"Kismet",
				"AdvancedPreviewScene",
				"SceneOutliner"
			}
		);
	}
}