// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

// Include
#include "GKBlueprintTraverse.h"

// Gamekit
#include "GKEdGraphVisitor.h"
#include "GKEdGraphTransform.h"
#include "GKEdGraphDebug.h"
#include "GKEdGraphUtils.h"

// UnrealEngine
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h" 
#include "Engine/SimpleConstructionScript.h"


void GeneratePythonFromBlueprint(class UBlueprintGeneratedClass* Source);
void GeneratePythonFromBlueprint(class USimpleConstructionScript* Source);

void GeneratePythonFromBlueprint(class UBlueprint* Source, FString Destination) {
    FGKEdGraphTransform Transformer(Source, Destination, Source->GetName());
    Transformer.Generate();
}

void GeneratePythonFromBlueprint(class UBlueprintGeneratedClass* Source) {
    GeneratePythonFromBlueprint(Source->SimpleConstructionScript);
}

void GeneratePythonFromBlueprint(class USimpleConstructionScript* Source) {
    Source->GetRootNodes();
}
