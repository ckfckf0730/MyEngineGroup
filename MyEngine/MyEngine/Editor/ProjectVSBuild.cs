using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Editor
{
    internal static class ProjectVSBuild
    {
        internal static void CreateProject(string path, string projectName)
        {

            string csprojPath = path + "/" + "Assembly-CSharp" + ".csproj";

            string slnPath = path + "/" + projectName + ".sln";

            CreateCsproj(csprojPath, path + "/");
            CreateSln(slnPath);
        }

        private static void CreateCsproj(string fullPath,string projectPath)
        {
            StringBuilder stringBuilder = new StringBuilder();

            stringBuilder.AppendLine("<?xml version=\"1.0\" encoding=\"utf-8\"?>");
            stringBuilder.AppendLine("<Project ToolsVersion=\"15.0\" DefaultTargets=\"Build\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">");

            stringBuilder.AppendLine("  <PropertyGroup>");
            stringBuilder.AppendLine("    <Configuration Condition=\" '$(Configuration)' == '' \">Debug</Configuration>");
            stringBuilder.AppendLine("    <Platform Condition=\" '$(Platform)' == '' \">AnyCPU</Platform>");
            stringBuilder.AppendLine("    <SchemaVersion>2.0</SchemaVersion>");
            stringBuilder.AppendLine("    <RootNamespace></RootNamespace>");
            stringBuilder.AppendLine("    <ProjectGuid>{}</ProjectGuid>");
            stringBuilder.AppendLine("    <OutputType>Library</OutputType>");
            stringBuilder.AppendLine("    <AppDesignerFolder>Properties</AppDesignerFolder>");
            stringBuilder.AppendLine("    <AssemblyName>Assembly-CSharp</AssemblyName>");
            stringBuilder.AppendLine("    <TargetFrameworkVersion>v4.7.2</TargetFrameworkVersion>");
            stringBuilder.AppendLine("    <FileAlignment>512</FileAlignment>");
            stringBuilder.AppendLine("    <BaseDirectory>.</BaseDirectory>");
            stringBuilder.AppendLine("  </PropertyGroup>");

            stringBuilder.AppendLine("  <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Debug|AnyCPU' \">");
            stringBuilder.AppendLine("    <DebugSymbols>true</DebugSymbols>");
            stringBuilder.AppendLine("    <DebugType>full</DebugType>");
            stringBuilder.AppendLine("    <Optimize>false</Optimize>");
            stringBuilder.AppendLine("    <OutputPath>Temp\\Bin\\Debug\\</OutputPath>");
            stringBuilder.AppendLine("    <DefineConstants>DEBUG;TRACE</DefineConstants>");
            stringBuilder.AppendLine("    <ErrorReport>prompt</ErrorReport>");
            stringBuilder.AppendLine("    <WarningLevel>4</WarningLevel>");
            //stringBuilder.AppendLine("    <NoWarn>0169</NoWarn>");
            stringBuilder.AppendLine("    <AllowUnsafeBlocks>False</AllowUnsafeBlocks>");
            stringBuilder.AppendLine("  </PropertyGroup>");

            stringBuilder.AppendLine("  <PropertyGroup Condition=\" '$(Configuration)|$(Platform)' == 'Release|AnyCPU' \">");
            stringBuilder.AppendLine("    <DebugType>pdbonly</DebugType>");
            stringBuilder.AppendLine("    <Optimize>true</Optimize>");
            stringBuilder.AppendLine("    <OutputPath>Temp\\bin\\Release\\</OutputPath>");
            stringBuilder.AppendLine("    <ErrorReport>prompt</ErrorReport>");
            stringBuilder.AppendLine("    <WarningLevel>4</WarningLevel>");
            //stringBuilder.AppendLine("    <NoWarn>0169</NoWarn>");
            stringBuilder.AppendLine("    <AllowUnsafeBlocks>False</AllowUnsafeBlocks>");
            stringBuilder.AppendLine("  </PropertyGroup>");

            stringBuilder.AppendLine("  <PropertyGroup>");
            stringBuilder.AppendLine("    <NoConfig>true</NoConfig>");
            stringBuilder.AppendLine("    <NoStdLib>true</NoStdLib>");
            stringBuilder.AppendLine("    <AddAdditionalExplicitAssemblyReferences>false</AddAdditionalExplicitAssemblyReferences>  ");
            stringBuilder.AppendLine("    <ImplicitlyExpandNETStandardFacades>false</ImplicitlyExpandNETStandardFacades>");
            stringBuilder.AppendLine("    <ImplicitlyExpandDesignTimeFacades>false</ImplicitlyExpandDesignTimeFacades>");
            stringBuilder.AppendLine("  </PropertyGroup>");

            //Find all scripts in projectPath/Assets/Scripts, and write them to <ItemGroup>
            string localPath = "Assets\\Scripts";
            var files = Directory.GetFiles(projectPath + localPath);
            bool isFirst = true;
            foreach(var file in files)
            {
                if(Path.GetExtension(file).ToLower() ==".cs")
                {
                    if (isFirst)
                    {
                        stringBuilder.AppendLine("  <ItemGroup>");
                        isFirst = false;
                    }

                    var fileName = Path.GetFileName(file);
                    string includePath = localPath + "\\" + fileName;
                    stringBuilder.AppendLine("    <Compile Include=\"" + includePath + "\" />");
                }
            }
            if(!isFirst)
            {
                stringBuilder.AppendLine("  </ItemGroup>");
            }

            stringBuilder.AppendLine("  <ItemGroup>");
            stringBuilder.AppendLine("    <Reference Include=\"System\" />");
            stringBuilder.AppendLine("    <Reference Include=\"System.Core\" />");
            stringBuilder.AppendLine("    <Reference Include=\"System.Numerics\" />");
            stringBuilder.AppendLine("    <Reference Include=\"System.Xml.Linq\" />");
            stringBuilder.AppendLine("    <Reference Include=\"System.Data.DataSetExtensions\" />");
            stringBuilder.AppendLine("    <Reference Include=\"Microsoft.CSharp\" />");
            stringBuilder.AppendLine("    <Reference Include=\"System.Data\" />");
            stringBuilder.AppendLine("    <Reference Include=\"System.Deployment\" />");
            stringBuilder.AppendLine("    <Reference Include=\"System.Drawing\" />");
            stringBuilder.AppendLine("    <Reference Include=\"System.Net.Http\" />");
            stringBuilder.AppendLine("    <Reference Include=\"System.Xml\" />");
            stringBuilder.AppendLine("  </ItemGroup>");

            stringBuilder.AppendLine("  <Import Project=\"$(MSBuildToolsPath)\\Microsoft.CSharp.targets\" />");
            stringBuilder.AppendLine("  <Target Name=\"GenerateTargetFrameworkMonikerAttribute\" />");

            stringBuilder.AppendLine("</Project>");

            var text = stringBuilder.ToString();

            File.WriteAllText(fullPath, text);
        }


        private static void CreateSln(string fullPath)
        {
            StringBuilder stringBuilder = new StringBuilder();

            stringBuilder.AppendLine("<?xml version=\"1.0\" encoding=\"utf-8\"?>");


            stringBuilder.AppendLine("Microsoft Visual Studio Solution File, Format Version 12.00");
            stringBuilder.AppendLine("# Visual Studio Version 17");

            stringBuilder.AppendLine("Project(\"{}\") = \"Assembly-CSharp\", \"Assembly-CSharp.csproj\", \"{}\"");
            stringBuilder.AppendLine("EndProject");

            stringBuilder.AppendLine("Global");
            stringBuilder.AppendLine("\tGlobalSection(SolutionConfigurationPlatforms) = preSolution");
            stringBuilder.AppendLine("\t\tDebug|Any CPU = Debug|Any CPU");
            stringBuilder.AppendLine("\t\tRelease|Any CPU = Release|Any CPU");
            stringBuilder.AppendLine("\tEndGlobalSection"); 
            stringBuilder.AppendLine("\tGlobalSection(ProjectConfigurationPlatforms) = postSolution");
            stringBuilder.AppendLine("\t\t{}.Debug|Any CPU.ActiveCfg = Debug|Any CPU");
            stringBuilder.AppendLine("\t\t{}.Debug|Any CPU.Build.0 = Debug|Any CPU");
            stringBuilder.AppendLine("\t\t{}.Release|Any CPU.ActiveCfg = Release|Any CPU");
            stringBuilder.AppendLine("\t\t{}.Release|Any CPU.Build.0 = Release|Any CPU");
            stringBuilder.AppendLine("\tEndGlobalSection");
            stringBuilder.AppendLine("\tGlobalSection(SolutionProperties) = preSolution");
            stringBuilder.AppendLine("\t\tHideSolutionNode = FALSE");
            stringBuilder.AppendLine("\tEndGlobalSection");
            stringBuilder.AppendLine("EndGlobal\r\n");

            var text = stringBuilder.ToString();

            File.WriteAllText(fullPath, text);
        }
    }
}
