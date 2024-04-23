using CkfEngine.Core;
using CkfEngine.Editor;
using CkfEngine.Forms;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Reflection.Emit;
using System.Runtime.InteropServices;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CkfEngine
{
    internal static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            Test();

            TitleForm titleForm = new TitleForm();
            Application.Run(titleForm);

            if (titleForm.IsOpenProject)
            {
                Application.Run(new Form1());
            }
        }


        static void Test()
        {
            AssemblyName assemblyName = new AssemblyName("MyAssembly");
            AssemblyBuilder assemblyBuilder = AssemblyBuilder.DefineDynamicAssembly(assemblyName, AssemblyBuilderAccess.Save);

            // 创建一个 ModuleBuilder
            ModuleBuilder moduleBuilder = assemblyBuilder.DefineDynamicModule("MyModule", "MyAssembly.dll");

            // 创建 TestB 类型
            TypeBuilder testBBuilder = moduleBuilder.DefineType("TestB", TypeAttributes.Class | TypeAttributes.Public);

            // 创建 TestB 类型的构造函数
            ConstructorBuilder constructorBBuilder = testBBuilder.DefineConstructor(MethodAttributes.Public, CallingConventions.Standard, Type.EmptyTypes);
            ILGenerator constructorBIl = constructorBBuilder.GetILGenerator();
            constructorBIl.Emit(OpCodes.Ret); // 空的构造函数

            // 创建 TestB 类型
            Type testBType = testBBuilder.CreateType();

            // 创建 TestA 类型
            TypeBuilder testABuilder = moduleBuilder.DefineType("TestA", TypeAttributes.Class | TypeAttributes.Public, typeof(TestB));

            // 创建 TestA 类型的构造函数
            ConstructorBuilder constructorABuilder = testABuilder.DefineConstructor(MethodAttributes.Public, CallingConventions.Standard, Type.EmptyTypes);
            ILGenerator constructorAIl = constructorABuilder.GetILGenerator();
            constructorAIl.Emit(OpCodes.Ret); // 空的构造函数

            // 创建 TestA 类型
            Type testAType = testABuilder.CreateType();

            // 保存程序集为 DLL 文件
            assemblyBuilder.Save("MyAssembly.dll");
        }

        


    }
    public class TestA : TestB
    {

    }

    public class TestB
    {

    }

}
