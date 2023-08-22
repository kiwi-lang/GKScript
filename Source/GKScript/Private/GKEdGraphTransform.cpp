// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

// Include
#include "GKEdGraphTransform.h"

// Gamekit
#include "GKEdGraphDebug.h"
#include "GKEdGraphUtils.h"

// Unreal Engine
#include "Misc/Paths.h"
#include "InputAction.h"

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

#define CAT_(a, b) a ## b
#define CAT(a, b) CAT_(a, b)

#define VARNAME(Var) CAT(Var, __LINE__)



#define GENPRINT(fmt, ...) Writer.Printf(TEXT(fmt), __VA_ARGS__)
#define WRITELINE(fmt, ...) Writer.Printf(TEXT("%s" fmt "\n"), *Indentation(), __VA_ARGS__)
#define INDENT() FGKIndentationGuard VARNAME(_GK_INDENTATION)(*this)
#define NEWSCOPE() FGKScopeGuard _GK_SCOPE(*this)
#define WRITENODETYPE(fmt, ...)         \
    if (bShowTypeName) {                \
        WRITELINE(fmt, __VA_ARGS__);    \
    }

FGKEdGraphTransform::FGKEdGraphTransform(class UBlueprint* Source, FString Folder, FString ScriptName):
    Source(Source) 
{
    IFileManager& FileManager = IFileManager::Get();
    FileManager.MakeDirectory(*Folder, true);

    Writer.OpenFile(Folder, ScriptName);
    IndentationLevel = 0;

    WRITELINE("import unreal");
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

    Context.Add(FGKGenContext());

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


    for (UEdGraph* Graph : Source->FunctionGraphs) {
        TArray<UK2Node*> Roots = FindRoots(Graph);
        for (UK2Node* Root : Roots) {
            Exec(Root);
        }
    }

    // This is generated trash, jsut calls the Ubergraph
    /*
    for (UEdGraph* Graph : Source->EventGraphs) {
        TArray<UK2Node*> Roots = FindRoots(Graph);
        for (UK2Node* Root : Roots) {
            Exec(Root);
        }
    }
    //*/

    for (UEdGraph* Graph : Source->UbergraphPages) {
        TArray<UK2Node*> Roots = FindRoots(Graph);
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
    if (Pin->PinName == FName("WorldContextObject")) {
        return "GetWorld()";
    }

    if (!Pin->DefaultValue.IsEmpty()) {
        if (Pin->DefaultValue.Contains(",")) {
            return FString::Printf(TEXT("(%s)"), *Pin->DefaultValue); 
        }
        return ToPythonValidValue(Pin->DefaultValue);
    }

    if (Pin->DefaultObject) {
        FString ClassName = Pin->DefaultObject->GetClass()->GetName();
        if (UBlueprintFunctionLibrary* Lib = Cast<UBlueprintFunctionLibrary>(Pin->DefaultObject)) {
            return ClassName;
        }
        return ClassName + "(\"" + Pin->DefaultObject->GetPathName() + "\")";
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



FGKResolvedPin FGKEdGraphTransform::ResolvePin(UEdGraphPin* StartPin, EEdGraphPinDirection Direction)
{
    FGKResolvedPin Result;
    Result.StartPin = StartPin;

    TArray<UEdGraphPin*> Pins;
    TMap<UEdGraphPin*, UEdGraphPin*> Visited;
    Pins.Add(StartPin);

    while (Pins.Num() > 0) {
        UEdGraphPin* Pin = Pins.Pop(false);

        // This is only possible if the root was not linked to anything in the first place
        if (Pin->LinkedTo.Num() == 0) {
            Result.Value = GetValue(Pin);
            break;
        }

        for (UEdGraphPin* Link : Pin->LinkedTo)
        {
            if (Link->Direction == Direction) {
                continue;
            }

            UEdGraphPin** WasVisited = Visited.Find(Link);
            if (WasVisited != nullptr) {
                Result.EndPin = WasVisited[0];
            }

            // Re route nodes
            UEdGraphNode* NextGraphNode = Link->GetOwningNode();
            if (Cast<UK2Node_Knot>(NextGraphNode)) {
                for (UEdGraphPin* NextPin : NextGraphNode->Pins) {
                    if (NextPin->PinType.PinCategory == "exec") {
                        continue;
                    }

                    if (NextPin->Direction == Direction) {
                        Pins.Add(NextPin);
                    }
                }
                continue;
            }

            else if (auto Self = Cast<UK2Node_Self>(NextGraphNode)) {
                Result.Value = FString(TEXT("self"));
                Result.EndPin = Link;
                break;
            }
            else if (auto Variable = Cast<UK2Node_VariableGet>(NextGraphNode))
            {
                Result.Value = "self." + Variable->GetVarNameString();
                Result.EndPin = Link;
                break;
            }
            else 
            {
                // Input is coming from a graph
                FString* Varname = PinToVariable.Find(Pin);

                // Graph was not traversed yet
                if (Varname == nullptr) {
                    Exec(Cast<UK2Node>(NextGraphNode));
                    Varname = PinToVariable.Find(Link);
                }

                Result.EndPin = Link;
                Result.Node = NextGraphNode;
                Result.Value = "Missing";

                if (Varname != nullptr){
                    Result.Value = Varname[0];
                }
            }
        }
    }
    return Result;
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



void FGKEdGraphTransform::GetInputOutputs(UK2Node* Node, FGKResolvedPin& Self, TArray<FGKResolvedPin>& Inputs, TArray<FString>& Outputs) {
    for (UEdGraphPin* Pin : Node->Pins) {
        // Ignore execution pins
        if (Pin->PinType.PinCategory == "execute") {
            continue;
        }

        if (Pin->PinType.PinCategory == "exec") {
            continue;
        }


        if (Pin->Direction == EGPD_Input) {
            if (Pin->PinName == "self") {
                Self = ResolvePin(Pin, Pin->Direction);
                continue;
            }

            // TODO: We need to generate the code for the Arguments
            auto Resolved = ResolvePin(Pin, Pin->Direction);
            Inputs.Add(Resolved);
        }
        else if (Pin->Direction == EGPD_Output) {
            FString Name = ResolveOutputPin(Pin);
            Outputs.Add(Name);
        }
    }
}

void FGKEdGraphTransform::GetInputOutputs(UK2Node* Node, UEdGraphPin*& Self, TArray<FString>& Inputs, TArray<FString>& Outputs) {
    for (UEdGraphPin* Pin : Node->Pins) {
        // Ignore execution pins
        if (Pin->PinType.PinCategory == "exec") {
            continue;
        }

        if (Pin->Direction == EGPD_Input) {
            if (Pin->PinName == "self") {
                Self = Pin;
                FString Name = ResolveInputPin(Pin);
                PinToVariable.Add(Pin, Name);
                continue;
            }

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
    CallFunction(*Node->GetFunctionName().ToString(), Node);
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
    WRITELINE("def On_%s(self):", *Node->GetFunctionName().ToString());
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
    FString VariableName = "Subsystem";

    UEdGraphPin* OutputPin = Node->GetResultPin();
    FString* Name = PinToVariable.Find(OutputPin);

    if (Name != nullptr) {
        return;
    }

    UObject* Subsystem = OutputPin->PinType.PinSubCategoryObject.Get();
    UClass* Klass = Cast<UClass>(Subsystem);

    WRITELINE("%s = GetSubsystem(ClassName=%s)", *VariableName, *Subsystem->GetName());

    PinToVariable.Add(OutputPin, VariableName);
    Super::Exec(Node->GetThenPin());
}


void FGKEdGraphTransform::CallFunction(FString FunctionName, UK2Node* Node) {
    if (PreviousNodes.Contains(Node)) {
        GKSCRIPT_WARNING(TEXT("Already called"));
        return;
    }
    PreviousNodes.Add(Node);

    TArray<FGKResolvedPin> ResolvedInputs;
    TArray<FString> Outs;

    // Generate the code to compute the arguments
    // ExecuteArguments(Node);

    // TODO: We need to generate the code for the Arguments
    FGKResolvedPin Self;
    GetInputOutputs(Node, Self, ResolvedInputs, Outs);

    TArray<FString> Args;
    for (auto& Input : ResolvedInputs) {

        FString ArgName = Input.StartPin->PinName.ToString();
        FString Type = GetType(Input.StartPin);

        Args.Add(GenerateCallArgument(ArgName, Type, Input.Value));
    }

    if (Self.StartPin != nullptr) {
        FString SelfType = "self";

        if (!Self.Value.IsEmpty()) {
            SelfType = Self.Value;
        }

        if (Self.StartPin->DefaultObject) {
            SelfType = Self.StartPin->DefaultObject->GetClass()->GetName();
        }

        FunctionName = FString::Printf(TEXT("%s.%s"), *SelfType, *FunctionName);
    }

    ensure(!FunctionName.IsEmpty());

    WRITENODETYPE("# CallFunction");
    FString ArgList;
    {
        if (Args.Num() > 0) {
            {
                INDENT();
                ArgList = "\n" + Indentation() + Join(FString(",\n") + Indentation(), Args) + "\n";
            }
            ArgList += Indentation();
        }
    }

    if (Outs.Num() > 0)
    {
        WRITELINE("%s = %s(%s)", *Join(", ", Outs), *FunctionName, *ArgList);
    }
    else {
        WRITELINE("%s(%s)", *FunctionName, *ArgList);
    }


    Exec(Node->GetThenPin());
}

void FGKEdGraphTransform::EnhancedInputAction(UK2Node_EnhancedInputAction* Node) {
    if (PreviousNodes.Contains(Node)) {
        GKSCRIPT_WARNING(TEXT("Already called"));
        return;
    }
    PreviousNodes.Add(Node);

    FString InputActionName = Node->InputAction->GetName();


    // MakeFunction(InputActionName, Node);

    TArray<UEdGraphPin*> PinArgs;
    TArray<FString> Args;
    Args.Add("self");
    Args.Add("TriggerEvent");

    for (auto Pin : Node->Pins) {
        if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory != "exec") {
            PinArgs.Add(Pin);
            Args.Add(Pin->PinName.ToString());

            PinToVariable.Add(Pin, Pin->PinName.ToString());
        }
    }
    
    //*
   
    WRITENODETYPE("# EnhancedInputAction");
    WRITELINE("def %s(%s):", *InputActionName, *Join(", ", Args));

    {
        INDENT();
        WRITELINE(DOCSTRING "%s" DOCSTRING, *FormatDocstring(Node->GetTooltipText().ToString()));

        // FInputActionValue UEnhancedPlayerInput::GetActionValue(TObjectPtr<const UInputAction> ForAction) const;
        // FInputActionValue
        WRITELINE("match %s:", TEXT("TriggerEvent"));

        for (auto Pin : Node->Pins) {
            if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory == "exec") {
                INDENT();
                WRITELINE("case \"%s\":", *Pin->PinName.ToString());
                INDENT();

                if (Pin->LinkedTo.Num() == 0) {
                    WRITELINE("pass");
                }
                else {
                    Exec(Pin);
                }
            }
        }

        Super::Exec(Node->GetThenPin());
        GENPRINT("\n");
    }
    //*/
}


void FGKEdGraphTransform::MacroInstance(UK2Node_MacroInstance* Node) {
    

    UEdGraph* MacroGraph = Node->GetMacroGraph();
    UBlueprint* MacroLibray = Node->GetBlueprint();

    FString MacroName;
    MacroGraph->GetName(MacroName);

    TArray<FGKResolvedPin> ResolvedInputs;
    for (auto Pin : Node->Pins) {
        if (Pin->Direction == EGPD_Input && Pin->PinName != FName("exec")) {
            ResolvedInputs.Add(ResolvePin(Pin, EGPD_Input));
        }
    }

    TArray<FString> Args;
    for (auto& In : ResolvedInputs) {

        FString ArgName = In.StartPin->PinName.ToString();
        FString Type = GetType(In.StartPin);
        Args.Add(GenerateCallArgument(ArgName, Type, In.Value));
    }

    // Macros are match
    // because they have multiple control flow
    WRITENODETYPE("# MacroInstance");
    WRITELINE("match %s(%s):", *MacroName, *Join(", ", Args));
    for (auto Pin : Node->Pins) {
        if (Pin->Direction == EGPD_Output){
            INDENT();
            WRITELINE("case \"%s\":", *Pin->PinName.ToString());
            INDENT();

            if (Pin->LinkedTo.Num() == 0) {
                WRITELINE("pass");
            } else {
                Exec(Pin);
            }
        }
    }
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


void FGKEdGraphTransform::MakeFunction(FString FunctionName, UK2Node* Node) {
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
    Arguments.Add("self");

    GetInputOutputs(Node, Inputs, Arguments);

    UK2Node_FunctionEntry* Entry = Cast<UK2Node_FunctionEntry>(Node);
    FString Tooltip = Node->GetTooltipText().ToString();

    // WRITELINE("# MakeFunction");
    WRITELINE("def %s(%s):", *FunctionName, *Join(", ", Arguments));

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
    Arguments.Add("self");

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

void FGKEdGraphTransform::IfThenElse(UK2Node_IfThenElse* Node)
{
    // Compute Condition
    FString Name = ResolveInputPin(Node->GetConditionPin());
    int CharPos = 0;
    if (Name.FindChar(' ', CharPos)) {
        Name = Name.LeftChop(Name.Len() - CharPos);
    }

    WRITENODETYPE("# IfThenElse");

    WRITELINE("if %s:", *Name);
    {
        INDENT();
        Exec(Node->GetThenPin());
    }

    if (Node->GetElsePin()->LinkedTo.Num() > 0) {
        WRITELINE("else:");
        {
            INDENT();
            Exec(Node->GetElsePin());
        }
    }
}


void FGKEdGraphTransform::SetVariableOnPersistentFrame(UK2Node_SetVariableOnPersistentFrame* Node) 
{
    CallFunction("SetVariableOnPersistentFrame", Node);
}

void FGKEdGraphTransform::VariableSet(UK2Node_VariableSet* Node) {
    // CallFunction("VariableSet", Node);

    FName VariableName = Node->VariableReference.GetMemberName();
    UEdGraphPin* Input = nullptr;
    UEdGraphPin* Output = nullptr;

    for(auto Pin: Node->Pins) {
        if (Pin->Direction == EGPD_Input && Pin->PinType.PinCategory != "exec") {
            Input = Pin;
        } else if (Pin->Direction == EGPD_Output && Pin->PinType.PinCategory != "exec")  {
            Output = Pin;
        }
    }

    FString Name = FString::Printf(TEXT("self.%s"), *VariableName.ToString());
    FString Variable = VariableName.ToString();
    WRITELINE("self.%s = %s", *Variable, TEXT("Whatever"));


    PinToVariable.Add(Output, Name);

    Exec(Node->GetThenPin());
}

void FGKEdGraphTransform::PromotableOperator(UK2Node_PromotableOperator* Node) {
    CallFunction("Op", Node);

}