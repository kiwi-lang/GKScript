// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

// Include
#include "GKEdGraphTransform.h"

// Gamekit
#include "GKEdGraphDebug.h"
#include "GKEdGraphUtils.h"

// Unreal Engine
#include "Misc/Paths.h"

// C lib
#include <cstdio>


#define DOCSTRING "\"\"\""

FGKCodeWriter::FGKCodeWriter(): FilePointer(nullptr) {}

void FGKCodeWriter::OpenFile(FString Folder, FString ScriptName) {
    FString ContentDir = FPaths::ProjectContentDir();
    FString FolderPath = FPaths::Combine(ContentDir, Folder);
    FString FilePath = FPaths::Combine(FolderPath, ScriptName + ".us");

    const char* c_filename = TCHAR_TO_UTF8(*FilePath);

    Close();

#ifdef __STDC_LIB_EXT1__
    FILE* fp = nullptr;
    if (fopen_s(fp, c_filename, "w") == 0) {
        FilePointer = fp;
    } else {
        GKSCRIPT_WARNING(TEXT("FilePath does not exist %s"), *FilePath);
    }

#else
    FILE* fp = fopen(c_filename, "w");
    if (!fp) {
        GKSCRIPT_WARNING(TEXT("FilePath does not exist %s"), *FilePath);
    }
    FilePointer = fp;
#endif
}

void FGKCodeWriter::Close() {
    if (FilePointer) {
        fclose((FILE*)FilePointer);
    }
}

void FGKCodeWriter::Write(const char* Bytes) {
    fprintf((FILE*)FilePointer, "%s", Bytes);
}

FGKCodeWriter::~FGKCodeWriter() {
    Close();
}


struct FGKIndentationGuard {
    FGKIndentationGuard(FGKEdGraphTransform& Transform) :
        Transform(Transform)
    {
        Transform.IndentationLevel += 1;
    }

    ~FGKIndentationGuard() {
        Transform.IndentationLevel -= 1;
    }

    FGKEdGraphTransform& Transform;
};


#define GENPRINT(fmt, ...) Writer.Printf(TEXT(fmt), __VA_ARGS__)
#define WRITELINE(fmt, ...) Writer.Printf(TEXT("%s" fmt "\n"), *Indentation(), __VA_ARGS__)
#define INDENT() FGKIndentationGuard _GK_INDENTATION(*this);

FGKEdGraphTransform::FGKEdGraphTransform(FString Folder, FString ScriptName){
    Writer.OpenFile(Folder, ScriptName);
    IndentationLevel = 0;
}


FString FGKEdGraphTransform::Indentation() const {
    return FString::ChrN(IndentationLevel * 2, ' ');
}

/*
* 
void FGKEdGraphTransform::CallFunction(UK2Node_CallFunction* Node) {}
void FGKEdGraphTransform::FunctionEntry(UK2Node_FunctionEntry* Node) {}
void FGKEdGraphTransform::VariableGet(UK2Node_VariableGet* Node) {}
void FGKEdGraphTransform::Event(UK2Node_Event* Node) {}
void FGKEdGraphTransform::DynamicCast(UK2Node_DynamicCast* Node) {}
void FGKEdGraphTransform::Self(UK2Node_Self* Node) {}
void FGKEdGraphTransform::GetSubsystem(UK2Node_GetSubsystem* Node) {}
void FGKEdGraphTransform::EnhancedInputAction(UK2Node_EnhancedInputAction* Node) {}
void FGKEdGraphTransform::MacroInstance(UK2Node_MacroInstance* Node) {}

*/


void FGKEdGraphTransform::CallFunction(UK2Node_CallFunction* Node)
{
    TArray<FString> Args;
    TArray<FString> Outs;

    // Generate the code to compute the arguments
    ExecuteArguments(Node);

    // TODO: We need to generate the code for the Arguments
    GetArgumentReturns(Node, Args, Outs);

    if (Outs.Num() > 0)
    {
        GENPRINT("  %s = %s(%s)\n", *Join(",", Outs), *Node->GetFunctionName().ToString(), *Join(",", Args));
    }
    else
    {
        GENPRINT("  %s(%s)\n", *Node->GetFunctionName().ToString(), *Join(", ", Args));
    }

    Super::Exec(FindNextExecutionNode(Node));
    return Return();
}

void FGKEdGraphTransform::ExecuteArguments(UK2Node* Node)
{
    for (auto Pin : Node->Pins)
    {
        // Ignore execution pins
        if (Pin->PinType.PinCategory == "exec")
        {
            continue;
        }

        if (Pin->Direction == EGPD_Input)
        {
            for (auto Link : Pin->LinkedTo)
            {
                auto GraphNode = Link->GetOwningNode();
                Super::Exec(Cast<UK2Node>(GraphNode));
            }
        }
    }
}

void FGKEdGraphTransform::DynamicCast(UK2Node_DynamicCast* Node)
{
    TArray<FString> Args;
    TArray<FString> Outs;

    // Generate the code to compute the arguments
    ExecuteArguments(Node);

    // TODO: We need to generate the code for the Arguments
    GetArgumentReturns(Node, Args, Outs);

    // Get the outputs
    GetNodeOutputs(Node, Outs);
    GENPRINT("    %s = Cast(%s, %s)\n", *Join(",", Outs), *Node->TargetType->GetName(), *Join(",", Args));

    // TODO: Handle the Cast Failed exec pin
    //
    // LogBCUtils: Display: >> Input Pins :
    // LogBCUtils: Display: >>> Name: '' exec(Links : 1)
    // LogBCUtils: Display: >>> Name: 'Object' object(Links : 1)
    // LogBCUtils: Display: >> Output Pins :
    // LogBCUtils: Display: >>> Name: '' exec(Links : 1)
    // LogBCUtils: Display: >>> Name: 'Cast Failed' exec(Links : 0)
    // LogBCUtils: Display: >>> Name: 'As Floating Health' object(Links : 2)
    // LogBCUtils: Display: >>> Name: 'Success' bool(Links : 0)

    Super::Exec(FindNextExecutionNode(Node));
    return Return();
}

void FGKEdGraphTransform::Event(UK2Node_Event* Node)
{
    WRITELINE("# Event");
    WRITELINE("def On_%s():", *Node->GetFunctionName().ToString());

    {
        INDENT();
        WRITELINE(DOCSTRING "%s" DOCSTRING, *Node->GetDesc());
        /*
        // Kepp traversing
        auto Next = FindNextExecutionNode(Node);
        // Event is not implemented
        if (Next == nullptr)
        {
            GENPRINT("    pass%s", "");
        }
        //
        */

        Super::Exec(Node->GetThenPin());
    }

    GENPRINT("\n");
}

void FGKEdGraphTransform::FunctionEntry(UK2Node_FunctionEntry* Node) {

    FName FunctionName;

    if (Node->CustomGeneratedFunctionName != NAME_None) {
        FunctionName = Node->CustomGeneratedFunctionName;
    } else {
        FunctionName = Node->FunctionReference.GetMemberName();
    }

    WRITELINE("# FunctionEntry");
    WRITELINE("def %s():", *FunctionName.ToString());

    {
        INDENT();
        WRITELINE(DOCSTRING "%s" DOCSTRING, *Node->GetDesc());

        auto Then = Node->GetThenPin();
        Super::Exec(Then);

        if (!Then) {
            WRITELINE("    pass");
        }

        GENPRINT("\n");
    }
}

void FGKEdGraphTransform::VariableGet(UK2Node_VariableGet* Node) {
    GENPRINT("%s", *Node->GetVarNameString());
}

void FGKEdGraphTransform::Self(UK2Node_Self* Node) {
    GENPRINT("self");
}

void FGKEdGraphTransform::GetSubsystem(UK2Node_GetSubsystem* Node) {
    // UEdGraphPin* Pin = Node->GetClassPin();
   
    GENPRINT("GetSubsystem(%s=%s)", TEXT(""), TEXT("ClassName"));
    Super::Exec(Node->GetExecPin());
}

void FGKEdGraphTransform::EnhancedInputAction(UK2Node_EnhancedInputAction* Node) {
    WRITELINE("# EnhancedInputAction");
    WRITELINE("def %s():", *Node->GetName());

    {
        INDENT();
        WRITELINE(DOCSTRING "%s" DOCSTRING, *Node->GetDesc());

        Super::Exec(Node->GetThenPin());
        GENPRINT("\n");
    }
}

void FGKEdGraphTransform::MacroInstance(UK2Node_MacroInstance* Node) {
    WRITELINE("# MacroInstance");
    
    WRITELINE("%s", Node->GetMacroGraph()->GetFName());
    Super::Exec(Node->GetThenPin());
}
