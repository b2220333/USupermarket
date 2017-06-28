// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class PlacingObjectsEditorTarget : TargetRules
{
	public PlacingObjectsEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
        ExtraModuleNames.Add("PlacingObjects");
    }

	//
	// TargetRules interface.
	//
 //   // Deprecated in 4.16
	//public override void SetupBinaries(
	//	TargetInfo Target,
	//	ref List<UEBuildBinaryConfiguration> OutBuildBinaryConfigurations,
	//	ref List<string> OutExtraModuleNames
	//	)
	//{
	//	OutExtraModuleNames.AddRange( new string[] { "PlacingObjects" } );
	//}
}
