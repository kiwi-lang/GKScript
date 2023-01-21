#pragma once

// UnrealEngine
#include "Commandlets/Commandlet.h"

// Generated
#include "GKScriptCommands.generated.h"

// d:\p4>Engine\Binaries\Win64\UE4Editor-Cmd.exe
// <Your Project Name> -run=CommandletSample
UCLASS()
class UGKScriptCommandlet : public UCommandlet
{
    GENERATED_BODY()

public:
    virtual int32 Main(const FString& Params) override;
};