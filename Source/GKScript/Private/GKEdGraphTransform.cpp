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



void GetThenPins(UEdGraphNode* Node, TArray<UEdGraphPin*> ExecOut) {
    ExecOut.Reset();

    for (UEdGraphPin* Pin : Node->Pins) {
        if (Pin->Direction == EGPD_Input)
            continue;

        if (Pin->PinType.PinCategory == FName("exec")) {
            ExecOut.Add(Pin);
        }
    }
}


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

    if (FilePointer) {
        GKSCRIPT_VERBOSE(TEXT(" - %s"), *FPaths::ConvertRelativePathToFull(FilePath));
    }
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

struct FGKScopeGuard {
    FGKScopeGuard(FGKEdGraphTransform& Transform) :
        Transform(Transform)
    {
        Transform.Context.Add(FGKGenContext());
    }

    ~FGKScopeGuard() {
        Transform.Context.Pop();
    }

    FGKEdGraphTransform& Transform;
};

FString ToPythonValidValue(FString Value) {
    static TMap<FString, FString> Map = {
        {TEXT("true"), TEXT("True")},
        {TEXT("false"), TEXT("False")}
    };

    FString* Result = Map.Find(Value.ToLower());

    if (Result != nullptr) {
        return Result[0];
    }

    return Value;
}

FString GetType(FEdGraphPinType& PinType) {
    UObject* Object = PinType.PinSubCategoryObject.Get();

    if (Object) {
        // The actual object of the CDO ?
        return Object->GetName();
    }
    else {
        // PinCategory: struct, object, bool
        return PinType.PinCategory.ToString();
    }

    return TEXT("Type");
}

FString GetType(UEdGraphPin* Pin) {
    return GetType(Pin->PinType);
}


#define GENPRINT(fmt, ...) Writer.Printf(TEXT(fmt), __VA_ARGS__)
#define WRITELINE(fmt, ...) Writer.Printf(TEXT("%s" fmt "\n"), *Indentation(), __VA_ARGS__)
#define INDENT() FGKIndentationGuard _GK_INDENTATION(*this)
#define NEWSCOPE() FGKScopeGuard _GK_SCOPE(*this)
#define WRITENODETYPE(fmt, ...)         \
    if (bShowTypeName) {                \
        WRITELINE(fmt, __VA_ARGS__);    \
    }

FGKEdGraphTransform::FGKEdGraphTransform(class UBlueprint* Source, FString Folder, FString ScriptName):
    Source(Source) 
{
    Writer.OpenFile(Folder, ScriptName);
    IndentationLevel = 0;
}


FString FGKEdGraphTransform::GenerateCallArgument(FString const& Name, FString const& Type, FString const& Value) {
    if (bDebugTypes){
        return FString::Printf(TEXT("%s: %s = %s"), *Name, *Type, *Value);
    }
    return FString::Printf(TEXT("%s = %s"), *Name, *Value);
}

FString FGKEdGraphTransform::GenerateReturnVariable(FString const& Name, FString const& Type) {
    if (bDebugTypes) {
        return FString::Printf(TEXT("%s: %s"), *Name, *Type);
    }
    return Name;
}

void FGKEdGraphTransform::Generate() {
    // Godot Like
#if 0
    WRITELINE(DOCSTRING "%s" DOCSTRING, *FormatDocstring(Source->BlueprintDescription));
    WRITELINE("");
    WRITELINE("extends(%s)", *Source->ParentClass->GetName());
    WRITELINE("");
    for(FBPVariableDescription& Variable: Source->NewVariables) {
        WRITELINE("%s: %s = DefaultSubObject(%s)",
            *Variable.VarName.ToString(),
            *GetType(Variable.VarType),
            *Variable.DefaultValue
        );
    }
    WRITELINE("");
    WRITELINE("");
#else
    // Python like
    WRITELINE("class %s(%s):", *Source->GetName(), *Source->ParentClass->GetName());
    INDENT();
    WRITELINE(DOCSTRING "%s" DOCSTRING, *FormatDocstring(Source->BlueprintDescription));
    WRITELINE("");
    for (FBPVariableDescription& Variable : Source->NewVariables) {
        WRITELINE("%s: %s = DefaultSubObject(%s)",
            *Variable.VarName.ToString(),
            *GetType(Variable.VarType),
            *Variable.DefaultValue
        );
    }
    WRITELINE("");
#endif

    TArray<UEdGraph*> Graphs;
    Source->GetAllGraphs(Graphs);

    for (UEdGraph* Graph : Graphs) {
        TArray<UK2Node*> Roots = FindRoots(Graph);

        if (Roots.Num() == 0) {
            GKSCRIPT_WARNING(TEXT("No root nodes found"));
        }

        for (UK2Node* Root : Roots) {
            Exec(Root);
        }
    }
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



FString FGKEdGraphTransform::MakeVariable(UEdGraphPin* Pin) {
    return ResolveOutputPin(Pin);
}

FString FGKEdGraphTransform::GetVariable(UEdGraphPin* Pin) {
    return ResolveInputPin(Pin);
}


FString GetValue(UEdGraphPin* Pin) {
    if (!Pin->DefaultValue.IsEmpty()) {
        if (Pin->DefaultValue.Contains(",")) {
            return FString::Printf(TEXT("(%s)"), *Pin->DefaultValue); 
        }
        return ToPythonValidValue(Pin->DefaultValue);
    }

    if (Pin->DefaultObject) {
        return FString("Object");
    }

    if (!Pin->DefaultTextValue.IsEmpty()) {
        return FString::Printf(TEXT("\"%s\""), *Pin->DefaultTextValue.ToString());
    }

    return FString();
}


TArray<FString> FGKEdGraphTransform::FindAllNames(UEdGraphPin* EndPin) {
    TSet<UEdGraphPin*> Visited;
    TSet<FString> Names;

    _FindAllNames(EndPin, Visited, Names);

    return Names.Array();
}

void FGKEdGraphTransform::_FindAllNames(UEdGraphPin* EndPin, TSet<UEdGraphPin*>& Visited, TSet<FString>& Names) {
    bool bAlreadyIn = false;
    Visited.Add(EndPin, &bAlreadyIn);
    if (bAlreadyIn) {
        return;
    }
    static TSet<FString> Forbidden = {
        TEXT("OutputPin"),
        TEXT("InputPin"),
        TEXT("ReturnValue"),
        TEXT("self"),
    };

    FString Name = EndPin->GetName();

    if (!Forbidden.Contains(Name)) {
        Names.Add(Name);
    }

    for(UEdGraphPin* Link: EndPin->LinkedTo) {
        
        UEdGraphNode* NextGraphNode = Link->GetOwningNode();
        if (auto Knot = Cast<UK2Node_Knot>(NextGraphNode)) {
            for(UEdGraphPin* Pin: Knot->Pins) {
                _FindAllNames(Pin, Visited, Names);
            }
        }
        else {
            _FindAllNames(Link, Visited, Names);
        }
    }
}

FString FGKEdGraphTransform::ResolveOutputPin(UEdGraphPin* EndPin) {
    FString* Result = PinToVariable.Find(EndPin);
    if (Result) {
        return Result[0];
    }

    // Find the names of the pins 
    TArray<FString> PinNames = FindAllNames(EndPin);

    FGKGenContext& CurrentContext = Context.Last();
    FString SelectedName;
    bool bUnique = false;

    for (FString const& Name : PinNames) {
        if (!CurrentContext.Variables.Contains(Name)) {
            SelectedName = Name;
            bUnique = true;
        }
    }

    if (PinNames.Num() == 1) {
        SelectedName = PinNames[0];
    } else if (PinNames.Num() == 0) {
        SelectedName = FString::Printf(TEXT("%s_%d"), *GetType(EndPin), CurrentContext.Variables.Num());
    }

    if (!bUnique) {
        int i = 0;
        FString NewName = SelectedName;
        while (CurrentContext.Variables.Contains(NewName))
        {
            NewName = FString::Printf(TEXT("%s_%d"), *SelectedName, CurrentContext.Variables.Num());
        } 
        bUnique = true;
    }

    CurrentContext.Variables.Add(SelectedName);
    PinToVariable.Add(EndPin, SelectedName);

    return GenerateReturnVariable(*SelectedName, *GetType(EndPin));
}

FString FGKEdGraphTransform::ResolveInputPin(UEdGraphPin* EndPin) {
    TArray<UEdGraphPin*> Pins;
    Pins.Add(EndPin);
    ensure(EndPin->Direction == EGPD_Input);

    FString ArgName = EndPin->PinName.ToString();
    FString Type = GetType(EndPin);

    while (Pins.Num() > 0) {
        UEdGraphPin* Pin = Pins.Pop(false);
        // GKSCRIPT_VERBOSE(TEXT("%s"), *PinName);

        // Default Value Pin
        if (Pin->LinkedTo.Num() == 0) {
            FString Value = GetValue(Pin);
            if (Value.IsEmpty()) {
                return GenerateCallArgument(ArgName, Type, FString(TEXT("MissingLink()")));
            }
            return GenerateCallArgument(ArgName, Type, Value);
        }

        for (UEdGraphPin* Link : Pin->LinkedTo)
        {
            // Downstream link
            if (Link->Direction == EGPD_Output) {

                FString* Result2 = PinToVariable.Find(Link);
                if (Result2 != nullptr) {
                    return GenerateCallArgument(ArgName, Type, Result2[0]);
                }

                UEdGraphNode* NextGraphNode = Link->GetOwningNode();
                if (Cast<UK2Node_Knot>(NextGraphNode)) {
                    for (UEdGraphPin* NextPin : NextGraphNode->Pins) {
                        if (NextPin->PinType.PinCategory == "exec") {
                            continue;
                        }

                        if (NextPin->Direction == EGPD_Input) {
                            Pins.Add(NextPin);
                        }
                    }
                    continue;
                } 
                else if (auto Self = Cast<UK2Node_Self>(NextGraphNode)) {
                    return GenerateCallArgument(ArgName, Type, FString(TEXT("self")));
                }
                else if (auto Variable = Cast<UK2Node_VariableGet>(NextGraphNode))
                {
                    return GenerateCallArgument(ArgName, Type, *Variable->GetVarNameString());
                }
                else 
                {
                    // We found a graph to call
                    FString* Result = PinToVariable.Find(Pin);

                    if (Result == nullptr) {
                        // Create a Variable using the output Node
                        Exec(Cast<UK2Node>(NextGraphNode));
                        Result = PinToVariable.Find(Link);
                    }

                    if (Result != nullptr) {
                        return GenerateCallArgument(ArgName, Type, Result[0]);
                    } else {
                        return GenerateCallArgument(ArgName, Type, FString(TEXT("MissingObject()")));
                    }
                    
                }
            }
        }
    }

    return GenerateCallArgument(ArgName, Type, FString(TEXT("?")));
}

void FGKEdGraphTransform::GetInputOutputs(UK2Node* Node, TArray<FString>& Inputs, TArray<FString>& Outputs) {
    for (UEdGraphPin* Pin : Node->Pins) {
        // Ignore execution pins
        if (Pin->PinType.PinCategory == "exec") {
            continue;
        }

        if (Pin->Direction == EGPD_Input) {
            // TODO: We need to generate the code for the Arguments
            FString Name = ResolveInputPin(Pin);

            Inputs.Add(Name);
        }
        else if (Pin->Direction == EGPD_Output) {
            FString Name = ResolveOutputPin(Pin);
            Outputs.Add(Name);
        }
    }
}

void FGKEdGraphTransform::CallFunction(UK2Node_CallFunction* Node)
{
    if (PreviousNodes.Contains(Node)) {
        GKSCRIPT_WARNING(TEXT("Already called"));
        return;
    }
    PreviousNodes.Add(Node);

    TArray<FString> Args;
    TArray<FString> Outs;

    // Generate the code to compute the arguments
    // ExecuteArguments(Node);

    // TODO: We need to generate the code for the Arguments
    GetInputOutputs(Node, Args, Outs);

    WRITENODETYPE("# CallFunction");
    if (Outs.Num() > 0)
    {
        WRITELINE("%s = %s(", *Join(", ", Outs), *Node->GetFunctionName().ToString());
    }
    else {
        WRITELINE("%s(", *Node->GetFunctionName().ToString());
    }

    for (FString& Arg : Args) {
        INDENT();
        WRITELINE("%s,", *Arg);
    }
    WRITELINE(")");


    Super::Exec(FindNextExecutionNode(Node));
    return Return();
}

void FGKEdGraphTransform::DynamicCast(UK2Node_DynamicCast* Node)
{
    TArray<FString> Args;
    TArray<FString> Outs;

    // TODO: We need to generate the code for the Arguments
    GetInputOutputs(Node, Args, Outs);

    // Get the outputs
    GetNodeOutputs(Node, Outs);
    WRITELINE("%s = Cast(%s, %s)", *Join(",", Outs), *Node->TargetType->GetName(), *Join(",", Args));

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
    WRITENODETYPE("# Event");
    WRITELINE("def On_%s():", *Node->GetFunctionName().ToString());
    NEWSCOPE();
    {
        INDENT();
        WRITELINE(DOCSTRING "%s" DOCSTRING, *FormatDocstring(Node->GetTooltipText().ToString()));

        Super::Exec(Node->GetThenPin());
    }

    GENPRINT("\n");
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
    WRITENODETYPE("# EnhancedInputAction");
    WRITELINE("def %s():", *Node->GetName());

    {
        INDENT();
        WRITELINE(DOCSTRING "%s" DOCSTRING, *FormatDocstring(Node->GetTooltipText().ToString()));

        Super::Exec(Node->GetThenPin());
        GENPRINT("\n");
    }
}


void FGKEdGraphTransform::MacroInstance(UK2Node_MacroInstance* Node) {
    WRITENODETYPE("# MacroInstance");
    
    TArray<FString> Inputs;
    TArray<FString> Outputs;

    // FIXME
    GetInputOutputs(Node, Inputs, Outputs);

    UEdGraph* MacroGraph = Node->GetMacroGraph();


    // Does not work ?
    // TArray<UK2Node*> Roots = FindRoots(MacroGraph);

    UK2Node* Root = nullptr;
    for(UEdGraphNode* MacroNode: MacroGraph->Nodes) {
        
        int InputPins = 0;
        int ExecPins = 0;

        for(UEdGraphPin* Pin: MacroNode->Pins) {
            InputPins += int(Pin->Direction == EGPD_Input);
            ExecPins += Pin->PinName == FName("exec");
        }

        bool bIsRoot = InputPins == 0 && ExecPins == 1;
        if (bIsRoot) {
            Root = Cast<UK2Node>(MacroNode);
            break;
        }
    }

    Super::Exec(Root);
    

}

void FGKEdGraphTransform::Tunnel(UK2Node_Tunnel* Node) {
    WRITENODETYPE("# Tunnel");
    TArray<FString> Inputs;
    TArray<FString> Outputs;
    GetInputOutputs(Node, Inputs, Outputs);

    auto Next = Node->GetThenPin();
    if (Next){
        Super::Exec(Cast<UK2Node>(Next->GetOwningNode()));
    }
}

// This is executed on
void FGKEdGraphTransform::Knot(UK2Node_Knot* Node) {

    int Count = 0;
    for (UEdGraphPin* Pin : Node->Pins) {
        if (Pin && Pin->Direction == EGPD_Output) {
            Super::ExecGraphNode(Pin->GetOwningNode());
            Count += 1;
        }
    }

    // We can only have one source
    ensure(Count == 1);
}


FString FGKEdGraphTransform::FormatDocstring(FString const& Docstring) {
    FString Indent = TEXT("\n") + Indentation();
    return Docstring.Replace(TEXT("\n"), *Indent);
}

void FGKEdGraphTransform::FunctionTerminator(UK2Node_FunctionTerminator* Node) 
{
    if (PreviousNodes.Contains(Node)) {
        GKSCRIPT_WARNING(TEXT("Inifinite Loop"));
        return;
    }
    PreviousNodes.Add(Node);

    NEWSCOPE();

    // Generate the code to compute the arguments
    // ExecuteArguments(Node);

    TArray<FString> Inputs;
    TArray<FString> Arguments;
    GetInputOutputs(Node, Inputs, Arguments);

    FName FunctionName = Node->FunctionReference.GetMemberName();

    UK2Node_FunctionEntry* Entry = Cast<UK2Node_FunctionEntry>(Node);
    FString Tooltip = Node->GetTooltipText().ToString();

    if (Entry) {
        if (Entry->CustomGeneratedFunctionName != NAME_None) {
            FunctionName = Entry->CustomGeneratedFunctionName;
        }
        Tooltip = Entry->MetaData.ToolTip.ToString();
    }

    // WRITELINE("# FunctionTerminator");
    WRITELINE("def %s(%s):", *FunctionName.ToString(), *Join(", ", Arguments));

    {
        INDENT();
        WRITELINE(DOCSTRING "%s" DOCSTRING, *FormatDocstring(Tooltip));

        auto Then = Node->GetThenPin();
        Super::Exec(Then);

        if (!Then) {
            WRITELINE("    pass");
        }

        GENPRINT("\n");
    }
}

// Return Node
void FGKEdGraphTransform::FunctionResult(UK2Node_FunctionResult* Node) 
{
    TArray<FString> Inputs;
    TArray<FString> Outs;
    GetInputOutputs(Node, Inputs, Outs);

    // 
    WRITENODETYPE("# FunctionResult");
    TArray<FString> Values;
    for(FString& Input: Inputs) {
        WRITELINE(TEXT("%s"), *Input);

        // Extract the variable names for the return statement
        for(int i = 0; i < Input.Len(); i++) {
            if (Input[i] == ' ' || Input[i] ==  ':') {
                Values.Add(Input.LeftChop(Input.Len() - i));
                break;
            }
        }
    }
    WRITELINE("return %s", *Join(", ", Values));
}

void FGKEdGraphTransform::FunctionEntry(UK2Node_FunctionEntry* Node)
{
    WRITENODETYPE("# FunctionEntry");
    FunctionTerminator(Node);
}