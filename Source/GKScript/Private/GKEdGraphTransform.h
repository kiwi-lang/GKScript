// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.
#pragma once

// Gamekit
#include "GKEdGraphVisitor.h"

// Unreal Engine
#include "Misc/Paths.h"

struct FGKCodeWriter {
    
    FGKCodeWriter();

    ~FGKCodeWriter();

    template <typename FmtType, typename... Types>
    void Printf(const FmtType& Fmt, Types... Args) {
        FString Output = FString::Printf(Fmt, Args...);
        Write(TCHAR_TO_UTF8(*Output));
    }

    void OpenFile(FString Folder, FString ScriptName);

    void Write(const char* Bytes);

    void Close();

    void* FilePointer = nullptr;
};


struct FGKGenContext {
    UEdGraphPin* EndPin;
    FString  ValueName;
    TSet<FString> Variables;
};

/*! Basic proof of concept turning Blueprint graph into code
 *
 * TODO:
 *  * Make sure temporary variables are uniques
 *  * Add more nodes
 * 
 * 
 * .. note::
 *    
 *    the main visitor iterates through nodes by following the execution
 *    thread.
 * 
 *    It is up to you to go through the input/output threads.
 * 
 *      
 */
struct FGKEdGraphTransform : public FGKEdGraphVisitor<FGKEdGraphTransform, void> {
    using Return = void;
    using Super = FGKEdGraphVisitor<FGKEdGraphTransform, void>;

    FGKEdGraphTransform(class UBlueprint* Source, FString Folder, FString ScriptName);

    void Generate();

    FString Indentation() const;

    void GetInputOutputs(UK2Node* Node, TArray<FString>& Args, TArray<FString>& Outs);

    FString MakeVariable(UEdGraphPin* Pin);
    FString GetVariable(UEdGraphPin* Pin);

    FString ResolveInputPin(UEdGraphPin* EndPin);
    FString ResolveOutputPin(UEdGraphPin* EndPin);

    void _FindAllNames(UEdGraphPin* EndPin, TSet<UEdGraphPin*>& Visited, TSet<FString>& Names);
    TArray<FString> FindAllNames(UEdGraphPin* EndPin);


    // Helpers
    // -------
    FString FormatDocstring(FString const& Docstring);
    FString GenerateCallArgument(FString const& Name, FString const& Type, FString const& Value);
    FString GenerateReturnVariable(FString const& Name, FString const& Type);

    // Generate Transform Functions
    // ---------------------------
    //
    // clang-format off
#define NODE(Name)\
    Return Name(class UK2Node_##Name* Node);

    UK2NODES(NODE)
#undef NODE
    // clang-format on

    bool                        bShowTypeName = true;
    bool                        bDebugTypes = false;
    class UBlueprint*           Source = nullptr;
    TArray<FGKGenContext>       Context;
    FGKCodeWriter               Writer;           // FileWriter
    TMap<UEdGraphPin*, FString> PinToVariable;    // Convert Pins to variables
    int                         IndentationLevel; // Used to generate python code
                                                  // with the right indentation
};
