// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

// Include
#include "GKScript.h"

DEFINE_LOG_CATEGORY(LogGKScript)

// Gamekit
#include "GKMenus.h"

// Unreal Engine
#include "Engine/Blueprint.h"
#include "Misc/Paths.h"
#include "Modules/ModuleManager.h"

#define LOCTEXT_NAMESPACE "FGKScriptModule"


void FGKScriptModule::StartupModule()
{
    ExtendContentBrowserAssetSelection();
}

void FGKScriptModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGKScriptModule, GKScript)