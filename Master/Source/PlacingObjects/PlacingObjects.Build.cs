// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.IO;

public class PlacingObjects : ModuleRules
{
    public PlacingObjects(ReadOnlyTargetRules Target) : base (Target)
    {
        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "UMG", "Slate", "SlateCore" });

        // Uncomment if you are using Slate UI
        PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // PublicIncludePaths.AddRange(new string[] { "Source/VictoryBPLibrary/Public" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");

        // To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true

        LoadMongoDB(Target); //load MongoDb libraries
    }

    // *** MONGODB Stuff

    private string BoostPath
    {
        get { return "boost_1_63_0"; }
    }

    private string MongoCPath
    {
        get { return "mongo-c-driver"; }
    }

    private string MongoCXXPath
    {
        get { return "mongo-cxx-driver"; }
    }
    private string ModulePath
    {
        get { return ModuleDirectory; }
    }

    private string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModulePath, "../../ThirdParty")); }
    }

    private void CopyToBinaries(string Filepath, ReadOnlyTargetRules Target)
    {
        string binariesDir = Path.Combine(ModulePath, "../../Binaries", Target.Platform.ToString());
        string filename = Path.GetFileName(Filepath);

        if (!Directory.Exists(binariesDir))
            Directory.CreateDirectory(binariesDir);

        if (!File.Exists(Path.Combine(binariesDir, filename)))
            File.Copy(Filepath, Path.Combine(binariesDir, filename), true);
    }

    private bool LoadMongoDB(ReadOnlyTargetRules Target)
    {
        bool isLibrarySupported = false;

        if (Target.Platform == UnrealTargetPlatform.Win64)
        {
            isLibrarySupported = true;

            string BoostLibPath = Path.Combine(ThirdPartyPath, BoostPath, "stage/lib");
            string MongoCLibPath = Path.Combine(ThirdPartyPath, MongoCPath, "lib");
            string MongoCXXLibPath = Path.Combine(ThirdPartyPath, MongoCXXPath, "lib");

            //PublicAdditionalLibraries.Add(Path.Combine(MongoCLibPath, "bson-1.0.lib"));
            //PublicAdditionalLibraries.Add(Path.Combine(MongoCLibPath, "mongoc-1.0.lib"));

            //PublicAdditionalLibraries.Add(Path.Combine(MongoCLibPath, "bson-static-1.0.lib"));
            //PublicAdditionalLibraries.Add(Path.Combine(MongoCLibPath, "mongoc-static-1.0.lib"));

            //PublicAdditionalLibraries.Add(Path.Combine(MongoCXXLibPath, "libbsoncxx.lib"));
            //PublicAdditionalLibraries.Add(Path.Combine(MongoCXXLibPath, "libmongocxx.lib"));

            PublicAdditionalLibraries.Add(Path.Combine(MongoCXXLibPath, "bsoncxx.lib"));
            PublicAdditionalLibraries.Add(Path.Combine(MongoCXXLibPath, "mongocxx.lib"));

            PublicLibraryPaths.Add(BoostLibPath);
            //PublicAdditionalLibraries.Add("boost_filesystem");

            //CopyToBinaries(Path.Combine(ThirdPartyPath, MongoCPath, "bin/concrt140.dll"), Target);
            //CopyToBinaries(Path.Combine(ThirdPartyPath, MongoCPath, "bin/msvcp140.dll"), Target);
            //CopyToBinaries(Path.Combine(ThirdPartyPath, MongoCPath, "bin/vcruntime140.dll"), Target);

            CopyToBinaries(Path.Combine(ThirdPartyPath, MongoCPath, "bin/libbson-1.0.dll"), Target);
            CopyToBinaries(Path.Combine(ThirdPartyPath, MongoCPath, "bin/libmongoc-1.0.dll"), Target);

            CopyToBinaries(Path.Combine(ThirdPartyPath, MongoCXXPath, "bin/bsoncxx.dll"), Target);
            CopyToBinaries(Path.Combine(ThirdPartyPath, MongoCXXPath, "bin/mongocxx.dll"), Target);
        }

        if (isLibrarySupported)
        {
            string BoostInclPath = Path.GetFullPath(Path.Combine(ThirdPartyPath, BoostPath));
            string MongoCInclBsonPath = Path.GetFullPath(Path.Combine(ThirdPartyPath, MongoCPath, "include", "libbson-1.0"));
            string MongoCInclMongoPath = Path.GetFullPath(Path.Combine(ThirdPartyPath, MongoCPath, "include", "libmongoc-1.0"));
            string MongoCXX_Mongo_InclPath = Path.GetFullPath(Path.Combine(ThirdPartyPath, MongoCXXPath, "include/mongocxx/v_noabi"));
            string MongoCXX_Bson_InclPath = Path.GetFullPath(Path.Combine(ThirdPartyPath, MongoCXXPath, "include/bsoncxx/v_noabi"));

            //include path
            PublicIncludePaths.Add(BoostInclPath);
            PublicIncludePaths.Add(MongoCInclBsonPath);
            PublicIncludePaths.Add(MongoCInclMongoPath);
            PublicIncludePaths.Add(MongoCXX_Mongo_InclPath);
            PublicIncludePaths.Add(MongoCXX_Bson_InclPath);

            //Definitions.Add(string.Format("MONGOCXX_STATIC"));
            //Definitions.Add(string.Format("BSONCXX_STATIC"));
            Definitions.Add(string.Format("BOOST_DISABLE_ABI_HEADERS"));
        }

        return isLibrarySupported;
    }
}
