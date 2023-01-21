// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

// Include
#include "GKEdGraphUtils.h"

// Unreal Engine
#include "K2Node.h"


FString MakeLegalName(FString name) {
    name.ReplaceInline(TEXT(" "), TEXT("_"));
    return name;
}

void GetNodeOutputs(class UK2Node* Node, TArray<FString>& Outs) {
    for (auto Pin : Node->Pins) {
        // Ignore execution pins
        if (Pin->PinType.PinCategory == "exec") {
            continue;
        }

        if (Pin->Direction == EGPD_Output) {
            Outs.Add(MakeLegalName(Pin->GetName()));
        }
    }
}
void GetArgumentReturns(UK2Node* Node, TArray<FString>& Args, TArray<FString>& Outs) {
    for (auto Pin : Node->Pins) {
        // Ignore execution pins
        if (Pin->PinType.PinCategory == "exec") {
            continue;
        }

        if (Pin->Direction == EGPD_Input) {
            // TODO: We need to generate the code for the Arguments
            Args.Add(MakeLegalName(Pin->GetName()));
        }
        else {
            Outs.Add(MakeLegalName(Pin->GetName()));
        }
    }
}

// Finds Next Node
UK2Node* FindNextExecutionNode(UK2Node* Node) {
    if (Node == nullptr) {
        return nullptr;
    }

    TArray<UK2Node*> NextNodes;

    // Find the next output pin
    for (UEdGraphPin* Pin : Node->Pins) {
        if (Pin->Direction != EGPD_Output) {
            continue;
        }

        if (Pin->PinType.PinCategory != "exec") {
            continue;
        }

        for (UEdGraphPin* Target : Pin->LinkedTo) {
            if (auto NextNode = Cast<UK2Node>(Target->GetOwningNode())) {
                NextNodes.Add(NextNode);
            }
        }
    }

    if (NextNodes.Num() > 1) {
        GKSCRIPT_DISPLAY(TEXT("Unsupported multi exec out %d"), NextNodes.Num());
    }

    if (NextNodes.Num() == 0) {
        return nullptr;
    }

    return NextNodes[0];
}


TArray<UK2Node*> FindRoots(UEdGraph* Graph) {
    TArray<UK2Node*> Roots;

    for (UEdGraphNode* GraphNode : Graph->Nodes) {
        auto K2Node = Cast<UK2Node>(GraphNode);
        if (K2Node == nullptr) {
            continue;
        }

        

        // Roots do not have input pints
        if ((K2Node->GetThenPin() != nullptr && K2Node->GetExecPin() == nullptr)) {
            Roots.Add(K2Node);
        }
    }

    return Roots;
}

FString Join(FString Sep, TArray<FString> Strings) {
    int Size = Sep.Len() * (Strings.Num() - 1);

    for (FString& String : Strings) {
        Size += String.Len();
    }

    FString Result;
    Result.Reserve(Size);

    for (int i = 0; i < Strings.Num() - 1; i++) {
        Result += Strings[i];
        Result += Sep;
    }

    if (Strings.Num() >= 1) {
        Result += Strings[Strings.Num() - 1];
    }
    return Result;
}

int CountInputPins(TArray<UEdGraphPin*> const& Pins) {
    int Count = 0;
    for (auto Pin : Pins) {
        if (Pin->Direction != EGPD_Input) {
            continue;
        }

        Count += 1;
    }

    return Count;
}