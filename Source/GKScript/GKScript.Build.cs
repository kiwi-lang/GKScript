// Copyright 2023 Mischievous Game, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;
using System.Diagnostics;
using EpicGames.Core;


public class GKScript : ModuleRules
{
    public GKScript(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory));

        // ... add other public dependencies that you statically link with here ...
        PublicDependencyModuleNames.AddRange(new string[] {
                "Core",
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "BlueprintGraph",

                // EnhancedInput
                "EnhancedInput",
                "InputBlueprintNodes",
        });

        // Version Info
        // ------------
        // Automatically set by the CI
        string GKScript_TAG = "v1.2.0";
        string GKScript_HASH = "fd5965b0a334ba5784929aac36716dbf0e9fb9fb";
        string GKScript_DATE = "2023-01-15 03:23:34 +0000";

        PublicDefinitions.Add("GKScript_TAG=" + GKScript_TAG);
        PublicDefinitions.Add("GKScript_COMMIT=" + GKScript_HASH);
        PublicDefinitions.Add("GKScript_DATE=" + GKScript_DATE);
    }
}
