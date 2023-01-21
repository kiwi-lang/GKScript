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
    TArray<UEdGraph*> Graphs;

    Source->GetAllGraphs(Graphs);

    FGKEdGraphTransform Transformer(Destination, Source->GetName());

    for (UEdGraph* Graph : Graphs) {
        TArray<UK2Node*> Roots = FindRoots(Graph);

        if (Roots.Num() == 0) {
            GKSCRIPT_WARNING(TEXT("No root nodes found"));
        }

        for (UK2Node* Root : Roots) {
            Transformer.Exec(Root);
        }
    }

    // Find Constructor
    UBlueprintGeneratedClass* Generated = Cast<UBlueprintGeneratedClass>(Source->GeneratedClass);
    GeneratePythonFromBlueprint(Generated);
}

void GeneratePythonFromBlueprint(class UBlueprintGeneratedClass* Source) {
    GeneratePythonFromBlueprint(Source->SimpleConstructionScript);
}

void GeneratePythonFromBlueprint(class USimpleConstructionScript* Source) {
    Source->GetRootNodes();
}
