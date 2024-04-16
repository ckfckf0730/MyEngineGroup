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

        internal static Assembly CompileScript(string path)
        {
           var code = File.ReadAllText(path);

            Assembly assembly= Assembly.GetExecutingAssembly();

            var compilerResults = CompileCode(code, assembly);
            if(compilerResults.Errors.Count >0)
            {
                Console.WriteLine("Compile Error: " + path);
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
