// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

// Include
#include "GKEdGraphDebug.h"

// Gamekit
#include "GKEdGraphUtils.h"

// Unreal Engine
#include "Engine/Blueprint.h"
#include "K2Node.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_SetVariableOnPersistentFrame.h"
#include "K2Node_VariableSet.h"
#include "K2Node_VariableGet.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "K2Node_DynamicCast.h"

//enum EGraphType
//{
//    GT_Function,
//    GT_Ubergraph,
//    GT_Macro,
//    GT_Animation,
//    GT_StateMachine,
//    GT_MAX,
//};

//  UEdGraph
//      - 
// 
// 
//      - UEdGraphNode -> UK2Node
//
// UMaterialGraph
//      - UEdGraphNode -> UMaterialGraphNode_Base

DEFINE_LOG_CATEGORY_STATIC(LogBCUtils, All, All);


// This looks like the root of the graph
// but it does not have pins ? 
void Print(UEdGraph* Graph) {
    UE_LOG(LogBCUtils, Display, TEXT(
        "Class Name  : %s\n"        // EdGraph
        " - Desc        : %s\n"     // 
        " - Detail      : %s\n"     // No_Detailed_Info_Specified
        " - Name        : %s\n"     // ReceiveTick
        " - ExportedName: %s\n"     // None
        " - FName       : %s\n"     // ReceiveTick
        " - FullName    : %s\n"     // EdGraph /Game/HacknSlash/HacknSlashCharater.HacknSlashCharater:ReceiveTick
        " - PathName    : %s"),     // Game/HacknSlash/HacknSlashCharater.HacknSlashCharater:ReceiveTick
        *Graph->GetClass()->GetName(),
        *Graph->GetDesc(),
        *Graph->GetDetailedInfo(),
        *Graph->GetName(),
        *Graph->GetExporterName().ToString(),
        *Graph->GetFName().ToString(),
        *Graph->GetFullName(),
        *Graph->GetPathName()
    );
}

void PrintPin(UEdGraphPin const* Pin) {
    UE_LOG(LogBCUtils, Display, TEXT(">>> Name: '%s' %s (Links: %d)"),
        *Pin->GetDisplayName().ToString(),
        *(Pin->PinType.PinCategory.ToString()),
        Pin->LinkedTo.Num()
    );
}

void PrintPins(TArray<UEdGraphPin*> const& Pins) {
    UE_LOG(LogBCUtils, Display, TEXT(">> Input Pins:"));
    for (auto Pin : Pins) {
        if (Pin->Direction != EGPD_Input) {
            continue;
        }
        PrintPin(Pin);
    }

    UE_LOG(LogBCUtils, Display, TEXT(">> Output Pins:"));
    for (auto Pin : Pins) {
        if (Pin->Direction != EGPD_Output) {
            continue;
        }
        PrintPin(Pin);
    }
}

void PrintFunctionEntry(UK2Node_FunctionEntry const* FunEntry) {
    UFunction* Fun = FunEntry->FindSignatureFunction();
    if (Fun == nullptr) {
        return;
    }

    // Fun->EventGraphFunction;
    UE_LOG(LogBCUtils, Display, TEXT(">> Fun Name: %s"), *Fun->GetName());
}

void PrintSetVariable(UK2Node_SetVariableOnPersistentFrame const* SetVariable) {
}

void Print(UK2Node const* Node);

void PrintCallFunction(UK2Node_CallFunction const* FunCall) {
    UE_LOG(LogBCUtils, Display, TEXT(">> Name: %s"), *FunCall->GetName());
    UE_LOG(LogBCUtils, Display, TEXT(">> Fun Name: %s"), *FunCall->GetFunctionName().ToString());

    auto Signature = FunCall->GetSignature();
    UE_LOG(LogBCUtils, Display, TEXT(">> Fun Signature: %s"), *Signature.ToString());

    UFunction* FunRef = FunCall->GetTargetFunction();
    if (FunRef != nullptr) {
        UE_LOG(LogBCUtils, Display, TEXT(">> FunRef Name: %s"), *FunRef->GetName());
    }
    else {
        UE_LOG(LogBCUtils, Display, TEXT(">> FunRef Name: No Target Function :("));
    }

    // EventNodeOut is only populated for custom events
    UEdGraphNode const* EventNodeOut = nullptr;
    UEdGraph* Result = FunCall->GetFunctionGraph(EventNodeOut);

    // LogBCUtils : Display: >> Result was null
    // LogBCUtils : Display: >> EventNodeOut was null
    // LogBCUtils : Display: >> Result Graph was empty
    // LogBCUtils : Display: >> Could not convert EventNodeOut to K2Node

    if (!Result) {
        UE_LOG(LogBCUtils, Display, TEXT(">> Result was null"));
    }

    if (!EventNodeOut) {
        UE_LOG(LogBCUtils, Display, TEXT(">> EventNodeOut was null"));
    }

    if (Result) {
        DumpGraph(Result);
    }
    else {
        UE_LOG(LogBCUtils, Display, TEXT(">> Result Graph was empty"));
    }

    if (UK2Node const* K2Node = Cast<UK2Node const>(EventNodeOut)) {
        Print(K2Node);
    }
    else {
        UE_LOG(LogBCUtils, Display, TEXT(">> Could not convert EventNodeOut to K2Node"));
    }
}

void Print(UK2Node const* Node) {
    UE_LOG(LogBCUtils, Display, TEXT("> Node Name: %s"), *Node->GetClass()->GetName());
    PrintPins(Node->Pins);

    if (auto FunctionEntry = Cast<UK2Node_FunctionEntry const>(Node)) {
        PrintFunctionEntry(FunctionEntry);
    }
    if (auto SetVariable = Cast<UK2Node_SetVariableOnPersistentFrame const>(Node)) {
        PrintSetVariable(SetVariable);
    }
    if (auto CallFun = Cast<UK2Node_CallFunction const>(Node)) {
        PrintCallFunction(CallFun);
    }
    UE_LOG(LogBCUtils, Display, TEXT("<"));
}


void DumpGraph(UEdGraph* Graph) {
    Print(Graph);

    // Event Graphs
    // ------------
    // 
    // It seems the 3 nodes below are always present.
    // I think the first one is just a definition
    // second one setup the arguments for the call
    // third calls the function which might be trully the code written in blueprints
    // 
    // Example 1:
    // Graph: ReceiveTick
    //  Node0: K2Node_FunctionEntry, ReceiveTick
    //      - Output Exec                           (Links: 1)
    //      - Output Delta Seconds float            (Links: 1)
    //  Node1: K2Node_SetVariableOnPersistentFrame
    //      - Input Exec                            (Links: 1)
    //      - Input KNode Event Deta Seconds float  (Links: 1)
    //      - Output Exec                           (Links: 1)
    //  Node2: K2Node_CallFunction (FunctionName: ExecuteUbergraph_HacknSlashCharater)
    //      - Input Exec                            (Links: 1)
    //      - Input Target Object                   (Links: 0)
    //      - Input Entry Point int                 (Links: 0)
    //      - Output Exec                           (Links: 0)
    //
    // Example 2:
    // Graph: InpActEvt_LeftMouseButton_K2Node_InputKeyEvent_0
    //  Node0: K2Node_FunctionEntry
    //      - Output Exec                           (Links: 1)
    //      - Output Key struct                     (Links: 1)
    //  Node1: K2Node_SetVariableOnPersistentFrame
    //      - Input Exec                            (Links: 1)
    //      - K2Node Input Key Event Key struct     (Links: 1)
    //      - Output Exec                           (Links: 1)
    //  Node2: K2Node_CallFunction (FunctionName: ExecuteUbergraph_HacknSlashController)
    //      - Input Exec                            (Links: 1)
    //      - Input Target Object                   (Links: 0)
    //      - Input Entry Point int                 (Links: 0)
    //      - Output Exec                           (Links: 0)
    //
    for (UEdGraphNode* GraphNode : Graph->Nodes) {
        auto K2Node = Cast<UK2Node>(GraphNode);
        if (K2Node == nullptr) {
            continue;
        }

        Print(K2Node);
    }
}

void TraverseGraph(UEdGraph* Graph) {
    auto Roots = FindRoots(Graph);

    for (UK2Node* GraphNode : Roots) {
        UE_LOG(LogBCUtils, Display, TEXT("===================="));
        TraverseGraph(GraphNode);
    }
}

void TraverseGraph(UK2Node* Node) {
    if (Node == nullptr) {
        return;
    }

    UE_LOG(LogBCUtils, Display, TEXT("> Node Name: %s"), *Node->GetClass()->GetName());

    // Find the next output pin
    for (UEdGraphPin* Pin : Node->Pins) {
        if (Pin->Direction != EGPD_Output) {
            continue;
        }

        if (Pin->PinType.PinCategory != "exec") {
            continue;
        }

        for (UEdGraphPin* Target : Pin->LinkedTo) {
            TraverseGraph(Cast<UK2Node>(Target->GetOwningNode()));
        }
    }
}


/*
    UEdGraphPin* ExecPin = K2Node->GetExecPin();
    if (ExecPin == nullptr) {
        continue;
    }

    for (UEdGraphPin* Link : ExecPin->LinkedTo) {
        UEdGraphNode* NextNode = Link->GetOwningNode();
        UE_LOG(LogBCUtils, Display, TEXT(">> Node Name: %s"), *NextNode->GetClass()->GetName());
    }
*/
