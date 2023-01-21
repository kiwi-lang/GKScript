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

/*! Basic proof of concept turning Blueprint graph into code
 *
 * TODO:
 *  * Make sure temporary variables are uniques
 *  * Add more nodes
 */
struct FGKEdGraphTransform : public FGKEdGraphVisitor<FGKEdGraphTransform, void> {
    using Return = void;
    using Super = FGKEdGraphVisitor<FGKEdGraphTransform, void>;

    void ExecuteArguments(UK2Node* Node);

    FGKEdGraphTransform(FString Folder, FString ScriptName);

    FGKCodeWriter Writer;

    int IndentationLevel;

    FString Indentation() const;

    // Generate Transform Functions
    // ---------------------------
    //
    // clang-format off
#define NODE(Name)\
    Return Name(class UK2Node_##Name* Node);

    UK2NODES(NODE)
#undef NODE
    // clang-format on
};
