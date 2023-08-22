// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

#pragma once

#include "GKPythonVisitor.h"


/* Transform a python script into Blueprint
 */
class FGKPythonTransform : public PythonASTVisitor {

    FGKPythonTransform(FString OutputPath, class UBlueprint* Dest=nullptr);

    // Expression
    int call(expr_ty node_, int depth)override;

    // Statement
    int classdef(stmt_ty node_, int depth)override;
    int functiondef(stmt_ty node_, int depth)override;
    int returnstmt(stmt_ty node_, int depth)override;
    int assign(stmt_ty node_, int depth) override;
    int ifstmt(stmt_ty node_, int depth) override;

    FString OutputPath;
    class UBlueprint* Destination;
    class UEdGraph* CurrentGraph;
    TMap<FName, UEdGraphPin*> ArgNameToPin;
};