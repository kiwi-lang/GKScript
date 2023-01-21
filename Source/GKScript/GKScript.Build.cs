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
        string GKSCRIPT_TAG = "v0.0.0";
        string GKSCRIPT_HASH = "nohash";
        string GKSCRIPT_DATE = "1900-01-01 01:01:01 +0000";

        PublicDefinitions.Add("GKSCRIPT_TAG=" + GKSCRIPT_TAG);
        PublicDefinitions.Add("GKSCRIPT_COMMIT=" + GKSCRIPT_HASH);
        PublicDefinitions.Add("GKSCRIPT_DATE=" + GKSCRIPT_DATE);
    }
}
