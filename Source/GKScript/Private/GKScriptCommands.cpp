
// Include
#include "GKScriptCommands.h"
 
// Gamekit
#include "GKScript.h"
#include "GKBlueprintTraverse.h"

// Unreal Engine
#include "AssetRegistry/AssetRegistryModule.h"
#include "Developer/AssetTools/Public/AssetToolsModule.h"
#include "UObject/UObjectGlobals.h"


#define GKSTR_1(x) #x
#define GKSTR(x) GKSTR_1(x)

void ShowVersionInfo();
void WaitReady();
UBlueprint* LoadBlueprint(FString BlueprintPath);


int32 UGKScriptCommandlet::Main(const FString& Params)
{
    WaitReady();

    GKSCRIPT_VERBOSE(TEXT("Parameters: %s"), *Params);
    ShowVersionInfo();

    FString Destination = "GKScript";
    FString DebugValue = TEXT("/Game/TopDown/Blueprints/BP_TopDownController.BP_TopDownController");
    FString BlueprintPath = DebugValue;

    // Parse Parameters
    FParse::Value(*Params, TEXT("Blueprint="), BlueprintPath);
    FParse::Value(*Params, TEXT("Destination="), Destination);
    //

    UBlueprint* Blueprint = LoadBlueprint(BlueprintPath);

    if (Blueprint) {
        GKSCRIPT_VERBOSE(TEXT(""));
        GKSCRIPT_VERBOSE(TEXT(">> Generating Code"));
        GKSCRIPT_VERBOSE(TEXT(" - Destination: %s"), *Destination);
        GeneratePythonFromBlueprint(Blueprint, Destination);
        GKSCRIPT_VERBOSE(TEXT(""));
        GKSCRIPT_VERBOSE(TEXT("<< Finished"));
        GKSCRIPT_VERBOSE(TEXT(""));
    } else {
        GKSCRIPT_ERROR(TEXT("Could not load blueprint %s"), *BlueprintPath);
    }

    // this will be useful for regenerating outdated scripts
    // SourceControlProvider = &ISourceControlModule::Get().GetProvider();
    // SourceControlProvider->Init();

    return 0;
}

UBlueprint* LoadBlueprint(FString BlueprintPath) {
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

    // FString FullBlueprintPath = FPaths::ProjectContentDir() / BlueprintPath;
    FString FullBlueprintPath = BlueprintPath;

    return Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), NULL, *FullBlueprintPath));
}

void ShowVersionInfo() {
    FString Tag = GKSTR(GKSCRIPT_TAG);
    FString Commit = GKSTR(GKSCRIPT_COMMIT);
    FString Date = GKSTR(GKSCRIPT_DATE);

    GKSCRIPT_VERBOSE(TEXT(" - GKSCRIPT_TAG   : %s"), *Tag);
    GKSCRIPT_VERBOSE(TEXT(" - GKSCRIPT_COMMIT: %s"), *Commit);
    GKSCRIPT_VERBOSE(TEXT(" - GKSCRIPT_DATE  : %s"), *Date);
}

void WaitReady()
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
}