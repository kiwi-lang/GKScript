// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "Modules/ModuleManager.h"


//*
DECLARE_LOG_CATEGORY_EXTERN(LogGKScript, Log, All);

#define GKSCRIPT_FATAL(Format, ...)   UE_LOG(LogGKScript, Fatal, Format, ##__VA_ARGS__)
#define GKSCRIPT_ERROR(Format, ...)   UE_LOG(LogGKScript, Error, Format, ##__VA_ARGS__)
#define GKSCRIPT_WARNING(Format, ...) UE_LOG(LogGKScript, Warning, Format, ##__VA_ARGS__)
#define GKSCRIPT_DISPLAY(Format, ...) UE_LOG(LogGKScript, Display, Format, ##__VA_ARGS__)
#define GKSCRIPT_LOG(Format, ...)     UE_LOG(LogGKScript, Log, Format, ##__VA_ARGS__)
#define GKSCRIPT_VERBOSE(Format, ...) UE_LOG(LogGKScript, Verbose, Format, ##__VA_ARGS__)
#define GKSCRIPT_VERYVERBOSE(Format, ...) UE_LOG(LogGKScript, VeryVerbose, Format, ##__VA_ARGS__)

DECLARE_STATS_GROUP(TEXT("GKScript"), STATGROUP_GKScript, STATCAT_Advanced);
//*/

class FGKScriptModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
