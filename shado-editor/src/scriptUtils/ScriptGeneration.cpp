#include <filesystem>
#include <fstream>
#include "ScriptGeneration.h"
#include "debug/Debug.h"
#include "script/ScriptManager.h"

namespace Shado::Script {

	static void generateSLNFile(const std::filesystem::path& path, const std::string& projectName);
	static void generateClientCSProj(const std::filesystem::path& path, const std::string& projectName);
	static void writeFile(const std::filesystem::path& path, const std::string& content);
	static void replace(std::string& str, const std::string& from, const std::string& to);

	void generateProject(const std::string& projectPath, const std::string& projectName) {

		using namespace std::filesystem;

		path path = projectPath;
		path /= projectName;

		directory_entry entry(path);
		if (!entry.exists()) {
			//SHADO_CORE_INFO("Project directory does not exist... Creating new one {0}", path.string());
			bool status = create_directories(path);
			SHADO_CORE_ASSERT(status, "Failed to create directory {0}", path.string());
		}

		// Create projects 
		create_directory(path / projectName);
		create_directories(path / projectName / "core");
		

		// Add stuff to project
		generateSLNFile(path, projectName);
		generateClientCSProj(path, projectName);

		// Set the default DLL path
		ScriptManager::setAssemblyDefaultPath((path / projectName / ("bin/Debug/net6.0/" + projectName + ".dll")).string());

		// In the future, copy the Script-core files maybe?
		std::string testCoreFile = R"(using System;

namespace ${ProjectNamespace}.core	{
	
	public class Test	{
		public static void SayHello()	{
			Console.WriteLine("hello from C#");
		}
	}
}		)";
		std::string projectNamespace = projectName;
		replace(projectNamespace, " ", "_");
		replace(testCoreFile, "${ProjectNamespace}", projectNamespace);
		writeFile(path / projectName / "core/Test.cs", testCoreFile);
	}

	static void generateSLNFile(const std::filesystem::path& path, const std::string& projectName) {

		std::string slnFile = R"(Microsoft Visual Studio Solution File, Format Version 12.00
# Visual Studio Version 17
VisualStudioVersion = 17.1.32407.343
MinimumVisualStudioVersion = 10.0.40219.1
Project("{FAE04EC0-301F-11D3-BF4B-00C04F79EFBC}") = "${ProjectName}", "${ProjectName}\${ProjectName}.csproj", "{43FE68F5-F4F2-4E85-9C69-31D209D4A9D4}"
EndProject
Global
	GlobalSection(SolutionConfigurationPlatforms) = preSolution
		Debug|Any CPU = Debug|Any CPU
		Release|Any CPU = Release|Any CPU
	EndGlobalSection
	GlobalSection(ProjectConfigurationPlatforms) = postSolution
		{43FE68F5-F4F2-4E85-9C69-31D209D4A9D4}.Debug|Any CPU.ActiveCfg = Debug|Any CPU
		{43FE68F5-F4F2-4E85-9C69-31D209D4A9D4}.Debug|Any CPU.Build.0 = Debug|Any CPU
		{43FE68F5-F4F2-4E85-9C69-31D209D4A9D4}.Release|Any CPU.ActiveCfg = Release|Any CPU
		{43FE68F5-F4F2-4E85-9C69-31D209D4A9D4}.Release|Any CPU.Build.0 = Release|Any CPU
	EndGlobalSection
	GlobalSection(SolutionProperties) = preSolution
		HideSolutionNode = FALSE
	EndGlobalSection
	GlobalSection(ExtensibilityGlobals) = postSolution
		SolutionGuid = {8D85139B-DC96-4D4E-9948-6B5EF856F991}
	EndGlobalSection
EndGlobal
		)";

		// Replace ${ProjectName}
		replace(slnFile, "${ProjectName}", projectName);

		// Write it to the proper place
		writeFile(path / (projectName + ".sln"), slnFile);
	}

	static void generateClientCSProj(const std::filesystem::path& path, const std::string& projectName) {
		std::string vsprojFile = R"V0G0N(<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <OutputType>Library</OutputType>
    <TargetFramework>net6.0</TargetFramework>
    <ImplicitUsings>enable</ImplicitUsings>
    <Nullable>enable</Nullable>
  </PropertyGroup>
</Project>
		)V0G0N";

		replace(vsprojFile, "${ProjectName}", projectName);
		writeFile(path / projectName / (projectName + ".csproj"), vsprojFile);
	}

	static void writeFile(const std::filesystem::path& path, const std::string& content) {
		std::ofstream myfile;
		myfile.open(path);
		myfile << content;
		myfile.close();
	}

	static void replace(std::string& str, const std::string& from, const std::string& to) {
		size_t pos;
		while ((pos = str.find(from)) != std::string::npos) {
			str.replace(pos, from.size(), to);
		}
	}
}
