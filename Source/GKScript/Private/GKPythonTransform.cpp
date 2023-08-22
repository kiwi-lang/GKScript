
// Include
#include "GKPythonTransform.h"

// Gamekit


// Unreal Engine
#include "EdGraph/EdGraphNode.h"
#include "EdGraphSchema_K2.h"
#include "Factories/BlueprintFactory.h"
#include "AssetToolsModule.h"


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

//

FGKPythonTransform::FGKPythonTransform(FString OutputPath, class UBlueprint* Dest):
    OutputPath(OutputPath), Destination(Dest)
{}

// Expression
int FGKPythonTransform::call(expr_ty node_, int depth) {
    //
}

FString PythonFString(PyObject* Object) {
    return FString(PyBytes_AsString(Object));
}

FName PythonFName(PyObject* Object) {
    return FName(PyBytes_AsString(Object));
}


// Statement
int FGKPythonTransform::classdef(stmt_ty node_, int depth) {
    // Nested classes are not supported
    ensure(Destination == nullptr);

    // Resolve BaseClass
    asdl_seq* Bases = node_->v.ClassDef.bases;
    ensure(asdl_seq_LEN(Bases) == 1);
    expr_ty BaseClass = (expr_ty) asdl_seq_GET(Bases, 0);
    ensure(BaseClass->kind == Name_kind);
    FString BaseClassName = PythonFString(BaseClass->v.Name.id);
    UClass* BaseClassType = FindObject<UClass>(ANY_PACKAGE, *BaseClassName);
    // ----

    // Get ready to instantiate the blueprint
    UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
    Factory->ParentClass = BaseClassType;
    Factory->BlueprintType = EBlueprintType::BPTYPE_Normal;
    Factory->bSkipClassPicker = true;

    // -----------------------------
    // Save the class as a blueprint
    identifier name_obj = node_->v.ClassDef.name;
    FString ClassName = FString(PyBytes_AsString(name_obj));

    const FString SavePackagePath = FPaths::GetPath(OutputPath);
    const FString SaveAssetName = FPaths::GetBaseFilename(ClassName);

    // 1 Class = 1 Blueprint
    IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
    Destination = Cast<UBlueprint>(
        AssetTools.CreateAsset(
            SaveAssetName,
            SavePackagePath,
            UBlueprint::StaticClass(),
            Factory
        )
    );

    UEdGraph* NewGraph = NewObject<UEdGraph>(
        Destination,
        NAME_None,
        RF_Transactional
    );
    NewGraph->Schema = UEdGraphSchema_K2::StaticClass();
    Destination->FunctionGraphs.Add(NewGraph);
    CurrentGraph = NewGraph;

    // Build the Blueprint Graph/Nodes
    auto result = visit(node_->v.ClassDef.body, depth);
    Destination = nullptr;
    return result;
}
int FGKPythonTransform::functiondef(stmt_ty node_, int depth) 
{
    ensure(CurrentGraph != nullptr);
    ensure(Destination != nullptr);



    // use decorators to know the type of graph to add
    node_->v.FunctionDef.decorator_list;



    UK2Node_FunctionTerminator* NewNode = NewObject<UK2Node_FunctionTerminator>(
        CurrentGraph,
        NAME_None, 
        RF_Transactional
    );
    CurrentGraph->AddNode(NewNode);

    // Arguments are input pins
    arguments_ty args = node_->v.FunctionDef.args;

    for (int i = 0; i < asdl_seq_LEN(args->posonlyargs); i++) {
        arg_ty elt = (arg_ty)asdl_seq_GET(args->posonlyargs, i);

        expr_ty annotation = elt->annotation;
        FName PinName = PythonFName(elt->arg);

        FEdGraphPinType PinType;
        /*
        PinType.PinCategory = ;
        PinType.PinSubCategory = ;
        PinType.PinSubCategoryObject = ;
        PinType.PinSubCategoryMemberReference = ;
        PinType.FEdGraphTerminalType = ;
        PinType.ContainerType = ;
        */

        UEdGraphPin* ArgPin = NewNode->CreatePin(
            EEdGraphPinDirection::EGPD_Input,
            PinType,
            PinName,
            i
        );
        ArgNameToPin[PinName] = ArgPin;
    }

    // Build the graph
    auto result = visit(node_->v.FunctionDef.body, depth);

    CurrentGraph = nullptr;
    return result;
}
int FGKPythonTransform::returnstmt(stmt_ty node_, int depth) {}
int FGKPythonTransform::assign(stmt_ty node_, int depth) {}
int FGKPythonTransform::ifstmt(stmt_ty node_, int depth) {}
