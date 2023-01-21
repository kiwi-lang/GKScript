
// Include
#include "GKScriptCommands.h"
 
// Gamekit
#include "GKScript.h"
#include "GKBlueprintTraverse.h"

// Unreal Engine
#include "AssetRegistry/AssetRegistryModule.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "UObject/UObjectGlobals.h"


int32 UGKScriptCommandlet::Main(const FString& Params)
{
    auto& AssetRegistryModule = FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
    auto& AssetRegistry = AssetRegistryModule.Get();

    while (!GEngine) {
        FPlatformProcess::Sleep(0.1f);
    }

    AssetRegistry.SearchAllAssets(true);
    while (AssetRegistry.IsLoadingAssets())
    {
        AssetRegistry.Tick(1.0f);
    }

    FPlatformProcess::Sleep(1);
    GKSCRIPT_VERBOSE(TEXT("Hello"));
#if 0
    // ConstructorHelpers cannot be called outside a constructor
    ConstructorHelpers::FObjectFinder<UBlueprint> TestingBlueprint(                                          //
        TEXT("/Script/Engine.Blueprint'/Game/TopDown/Blueprints/BP_TopDownController.BP_TopDownController'") //
    );                                                                                                       //

    if (TestingBlueprint.Succeeded()) {
        GeneratePythonFromBlueprint(TestingBlueprint.Object);
    }
#endif
    // FStringAssetReference BlueprintRef = "Blueprints/MyBlueprint.MyBlueprint";
    // UBlueprint* Blueprint = Cast<UBlueprint>(BlueprintRef.TryLoad());


    FString BlueprintPath = TEXT("/Game/TopDown/Blueprints/BP_TopDownController.BP_TopDownController");

    GKSCRIPT_VERBOSE(TEXT("%s"), *Params);

    // FString FullBlueprintPath = FPaths::ProjectContentDir() / BlueprintPath;
    FString FullBlueprintPath = BlueprintPath;

    UBlueprint* Blueprint = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), NULL, *FullBlueprintPath));

    if (Blueprint) {
        GeneratePythonFromBlueprint(Blueprint);
    }

    // this will be useful for regenerating outdated scripts
    // SourceControlProvider = &ISourceControlModule::Get().GetProvider();
    // SourceControlProvider->Init();
    Done = true;

    return 0;
}