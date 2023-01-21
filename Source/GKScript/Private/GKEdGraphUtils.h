// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

#pragma once

// Unreal Engine
#include "CoreMinimal.h"

class FString;

void GetNodeOutputs(class UK2Node* Node, TArray<FString>& Outs);

UK2Node* FindNextExecutionNode(UK2Node* Node);

TArray<class UK2Node*> FindRoots(class UEdGraph* Graph);

FString Join(FString Sep, TArray<FString> Strings);

int CountInputPins(TArray<class UEdGraphPin*> const& Pins);
