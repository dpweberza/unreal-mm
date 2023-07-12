// Fill out your copyright notice in the Description page of Project Settings.

using System.IO;
using UnrealBuildTool;

public class CryptoPP : ModuleRules
{

	public CryptoPP(ReadOnlyTargetRules Target) : base(Target)
	{
		Type = ModuleType.External;

		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			// Add the import library
			PublicAdditionalLibraries.Add(Path.Combine(ModuleDirectory, "x64", "Release", "cryptlib.lib"));
			PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "include"));
		}

		bEnableUndefinedIdentifierWarnings = false;
		bUseRTTI = true;
	}
}
