// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.
#pragma once

// Unreal Engine
#include "CoreMinimal.h"

void Print(class UEdGraph* Graph);

void PrintPin(class UEdGraphPin const* Pin);

void PrintPins(TArray<class UEdGraphPin*> const& Pins);

void PrintFunctionEntry(class UK2Node_FunctionEntry const* FunEntry);

void PrintSetVariable(class UK2Node_SetVariableOnPersistentFrame const* SetVariable);

void Print(class UK2Node const* Node);

void PrintCallFunction(class UK2Node_CallFunction const* FunCall);

void Print(class UK2Node const* Node);

void DumpGraph(class UEdGraph* Graph);

void TraverseGraph(class UEdGraph* Graph);

void TraverseGraph(class UK2Node* Node);
