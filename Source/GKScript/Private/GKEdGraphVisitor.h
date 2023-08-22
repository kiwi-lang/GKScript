// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

#pragma once

// Gamekit
#include "GKScript.h"

// Unreal Engine
#include "K2Node.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_SetVariableOnPersistentFrame.h"
#include "K2Node_VariableSet.h"
#include "K2Node_VariableGet.h"
#include "K2Node_CallFunction.h"
#include "K2Node_Event.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_Knot.h"
#include "K2Node_Self.h"
#include "K2Node_GetSubsystem.h"
#include "K2Node_MacroInstance.h"
#include "K2Node_EnhancedInputAction.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_FunctionTerminator.h"
#include "K2Node_Tunnel.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_VariableSet.h"
#include "K2Node_PromotableOperator.h"

template <typename Impl, typename Return, typename... Args>
struct FGKEdGraphVisitor {
    
#define UK2NODES(NODE)\
    NODE(CallFunction)\
    NODE(VariableGet)\
    NODE(Event)\
    NODE(DynamicCast)\
    NODE(Self)\
    NODE(GetSubsystem)\
    NODE(EnhancedInputAction)\
    NODE(MacroInstance)\
    NODE(Knot)\
    NODE(FunctionTerminator)\
    NODE(FunctionResult)\
    NODE(FunctionEntry)\
    NODE(Tunnel)\
    NODE(IfThenElse)\
    NODE(SetVariableOnPersistentFrame)\
    NODE(VariableSet)\
    NODE(PromotableOperator)

    enum class NodeKind {
        Unknown,
        // Generate Node Enums
        // -------------------
        //
        // clang-format off
        #define NODE(Name) Name,
            UK2NODES(NODE)
        #undef NODE
        // clang-format on
    };

    const NodeKind ClassNodeTypeMapping(UK2Node* Node) {
        static TMap<UClass*, NodeKind> Mapping{
            // clang-format off
            // Generate Node class to Enum mapping
            // -----------------------------------
            #define NODE(Name)\
               { UK2Node_##Name::StaticClass(), NodeKind::Name },
            
                UK2NODES(NODE)

            #undef NODE
            // clang-format on
        };

        UClass* Class = Node->GetClass();
        if (!Mapping.Contains(Class)) {
            return NodeKind::Unknown;
        }

        return Mapping[Class];
    }

    // Traverse the output pins
    Return Exec(class UEdGraphPin* Pin, Args... args) {
        // ? Shouldn't it be Exex(Pin->GetOwningNode()) ?
        if (!Pin) {
            return Return();
        }

        for(class UEdGraphPin* OutPin: Pin->LinkedTo) {
            UEdGraphNode* Node = OutPin->GetOwningNode();

            if (UK2Node_Knot* Knot = Cast<UK2Node_Knot>(Node)) {
                for(UEdGraphPin* KnotPin: Knot->Pins){
                    if (KnotPin->Direction == EGPD_Output) {
                        Exec(KnotPin, args...);
                    }
                }
            
            } else {
                ExecGraphNode(Node, args...);
            }
        }

        return Return();
    }

    Return ExecGraphNode(UEdGraphNode* Node, Args... args) { 
        UK2Node* K2Node = Cast<UK2Node>(Node);

        if (K2Node) { 
            return Exec(K2Node, args...);
        }

        GKSCRIPT_DISPLAY(TEXT("Unreachable %s (Cast Error should be imposible)"), *Node->GetClass()->GetName());
        return Return();
    }

    struct FGKDepthGuard {
        FGKDepthGuard(FGKEdGraphVisitor& Transform) :
            Visitor(Transform)
        {
            Visitor.Depth += 1;
        }

        ~FGKDepthGuard() {
            Visitor.Depth -= 1;
        }

        FGKEdGraphVisitor& Visitor;
    };

    FString GetDepthViz() const {
        if (Depth == 1) {
            GKSCRIPT_VERBOSE(TEXT(""));
        }
        FString Temp = FString::ChrN(Depth - 1, ' ');
        for (int32 Cx = 0; Cx < Depth - 1; ++Cx)
        {
            Temp[Cx] = Cx & 1 ? ':' : '|';
        }
        return Temp;
    }

    // Exec find the underlying type of a UK2 node and call the correct implementation
    Return Exec(UK2Node* Node, Args... args) {
        FGKDepthGuard DepthGuard(*this);
        
        if (Node == nullptr) {
            return Return();
        }

        switch (ClassNodeTypeMapping(Node)) {

        // Generate Static dispatch
        // ------------------------
        //
        // clang-format off
        #define NODE(Name)\
            case NodeKind::Name: {\
                if (auto NodeCast = Cast<UK2Node_##Name>(Node)) {\
                    GKSCRIPT_VERBOSE(TEXT("%s+-> %s"), *GetDepthViz(), TEXT(#Name))\
                    return static_cast<Impl&>(*this).Name(NodeCast, args...);\
                }\
            }
            UK2NODES(NODE)

        #undef NODE
        // clang-format on

        // Edge cases
        case NodeKind::Unknown: {
            GKSCRIPT_DISPLAY(TEXT("%s-> Unknown class %s"), *GetDepthViz(), *Node->GetClass()->GetName());
            return Return();
        }
        }

        GKSCRIPT_DISPLAY(TEXT("%s-> Unreachable %s (Cast Error should be imposible)"), *GetDepthViz(), *Node->GetClass()->GetName());
        return Return();
    }

    // Generate Fallback functions
    // ---------------------------
    //
    // clang-format off
    #define NODE(Name)\
    Return Name(UK2Node_##Name* Node, Args... args) {\
        GKSCRIPT_WARNING(TEXT("%s was not implemented"), #Name);\
        return Return();\
    }

    UK2NODES(NODE)
    #undef NODE
    // clang-format on

    int Depth = 0;
    TSet<UEdGraphNode*> PreviousNodes;
};

