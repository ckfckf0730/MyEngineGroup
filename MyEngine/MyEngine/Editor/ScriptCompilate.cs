using System;
using System.CodeDom.Compiler;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Reflection;
using Microsoft.CSharp;
using System.IO;

namespace CkfEngine.Editor
{
    internal static class ScriptCompilate
    {
        internal static Dictionary<string, Type> ScriptTable = new Dictionary<string, Type>(); 

        internal static void CompileAllScript()
        {
            if( ProjectManager.Instance.CurProject == null )
            {
                return;
            }

            var proPath = ProjectManager.Instance.CurProject.Path;
            var files = Directory.GetFiles(proPath + "Assets/Scripts");
            foreach(var file in files)
            {
                if(Path.GetExtension(file) == ".cs")
                {
                    ScriptCompilate.CompileScript(file);
                }
            }

            EditorEvents.ScriptsOnCompiled?.Invoke(ScriptTable);
        }


        internal static Assembly CompileScript(string path)
        {
           var code = File.ReadAllText(path);

            Assembly engineAssembly= Assembly.GetExecutingAssembly();

            var compilerResults = CompileCode(code, engineAssembly);
            if(compilerResults.Errors.Count >0)
            {
                Console.WriteLine("Compile Error: " + path);
                foreach (CompilerError error in compilerResults.Errors)
                {
                    Console.WriteLine(error.ErrorText);
                }
            }

            var assembly = compilerResults.CompiledAssembly;
            var types = assembly.GetTypes();
            foreach(var type in types)
            {
                if(ScriptTable.ContainsKey(type.FullName))
                {
                    ScriptTable[type.FullName] = type;
                }
                else
                {
                    ScriptTable.Add(type.FullName, type);
                }

            }


            return compilerResults.CompiledAssembly;
        }

        internal static CompilerResults CompileCode(string sourceCode, params Assembly[] assemblies)
        {
            CSharpCodeProvider provider = new CSharpCodeProvider();
            CompilerParameters parameters = new CompilerParameters();

            parameters.GenerateExecutable = false;
            parameters.GenerateInMemory = true;

            foreach(Assembly assembly in assemblies)
            {
                parameters.ReferencedAssemblies.Add(assembly.Location);
            }

            parameters.ReferencedAssemblies.Add("System.dll");
            parameters.ReferencedAssemblies.Add("System.Core.dll");
            parameters.ReferencedAssemblies.Add("System.Numerics.dll");
            

            return provider.CompileAssemblyFromSource(parameters, sourceCode); 
        }
    }
}
