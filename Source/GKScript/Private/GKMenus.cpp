// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

// Include
#include "GKMenus.h"

// Gamekit
#include "GKScript.h"
#include "GKBlueprintTraverse.h"

// Unreal Engine
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"


#define LOCTEXT_NAMESPACE "FGKScriptModule"

void GenerateStructsForSelectedBlueprints(const TArray<FAssetData> SelectedAssets, bool bGenerateNativeStruct) {
	for (const FAssetData& AssetData : SelectedAssets) {
		UBlueprint* Blueprint = Cast<UBlueprint>(AssetData.GetAsset());
		if (Blueprint && Blueprint->GeneratedClass)
		{
			GeneratePythonFromBlueprint(Blueprint);
		}
	}
}

void AddBlueprintCodeActionMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets) {
	MenuBuilder.AddMenuEntry(
		LOCTEXT("GenerateCode", "Generate code From Blueprint"),
		LOCTEXT("GenerateCodeTooltip", "Generate code From Blueprint"),
		FSlateIcon(),
		FUIAction(FExecuteAction::CreateStatic(GenerateStructsForSelectedBlueprints, SelectedAssets, false), EUIActionRepeatMode::RepeatEnabled));
}

TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets) {
	TSharedRef<FExtender> Extender(new FExtender());

	bool bHaveAnyBlueprints = false;
	for (const FAssetData& AssetData : SelectedAssets) {
		bHaveAnyBlueprints |= AssetData.GetClass()->IsChildOf(UBlueprint::StaticClass());;
	}

	GKSCRIPT_DISPLAY(TEXT("Found Blueprints: %d"), bHaveAnyBlueprints);

	if (bHaveAnyBlueprints) {
		Extender->AddMenuExtension(
			"GetAssetActions",
			EExtensionHook::After,
			nullptr,
			FMenuExtensionDelegate::CreateStatic(&AddBlueprintCodeActionMenu, SelectedAssets));
	}

	return Extender;
}

void ExtendContentBrowserAssetSelection() {
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
	TArray<FContentBrowserMenuExtender_SelectedAssets>& 
		CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();

	CBMenuExtenderDelegates.Add(
		FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&OnExtendContentBrowserAssetSelectionMenu)
	);
}


#undef LOCTEXT_NAMESPACE