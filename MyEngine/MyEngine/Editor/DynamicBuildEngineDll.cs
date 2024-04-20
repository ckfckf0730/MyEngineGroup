using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection.Emit;
using System.Reflection;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Security.Cryptography;
using CkfEngine.Core;

namespace CkfEngine.Editor
{
    internal static class DynamicBuildEngineDll
    {
        internal static void BuildCore(string path)
        {
            AssemblyName assemblyName = new AssemblyName("CkfEngine.Core");
            AppDomain currentDom = Thread.GetDomain();
            AssemblyBuilder assemblyBuilder = currentDom.DefineDynamicAssembly(assemblyName, AssemblyBuilderAccess.Save, path);

            // create module
            ModuleBuilder moduleBuilder = assemblyBuilder.DefineDynamicModule("CkfEngine.Core", "CkfEngine.Core.dll");

            // create class 
            TypeBuilder typeBuilder = moduleBuilder.DefineType("CkfEngine.Core.EngineObject", TypeAttributes.Public | TypeAttributes.Serializable);

            // 添加 [Serializable] 特性
            ConstructorInfo serializableCtor = typeof(SerializableAttribute).GetConstructor(Type.EmptyTypes);
            CustomAttributeBuilder serializableBuilder = new CustomAttributeBuilder(serializableCtor, new object[] { });
            typeBuilder.SetCustomAttribute(serializableBuilder);

            // 添加字段 m_name 和 m_uid
            FieldBuilder fieldName = typeBuilder.DefineField("m_name", typeof(string), FieldAttributes.Private);
            FieldBuilder fieldUid = typeBuilder.DefineField("m_uid", typeof(ulong), FieldAttributes.Private);

            // 添加属性 Name 和 Uid
            PropertyBuilder propertyUid = typeBuilder.DefineProperty("Name", PropertyAttributes.None, typeof(string), null);
            MethodBuilder methodUidGet = typeBuilder.DefineMethod("get_Name", MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.HideBySig, typeof(string), Type.EmptyTypes);
            ILGenerator methodUidGetIL = methodUidGet.GetILGenerator();
            methodUidGetIL.Emit(OpCodes.Ldarg_0);
            methodUidGetIL.Emit(OpCodes.Ldfld, fieldName);
            methodUidGetIL.Emit(OpCodes.Ret);
            propertyUid.SetGetMethod(methodUidGet);

            PropertyBuilder propertyName = typeBuilder.DefineProperty("Uid", PropertyAttributes.None, typeof(ulong), null);
            MethodBuilder methodUidGet2 = typeBuilder.DefineMethod("get_Uid", MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.HideBySig, typeof(ulong), Type.EmptyTypes);
            ILGenerator methodUidGetIL2 = methodUidGet2.GetILGenerator();
            methodUidGetIL2.Emit(OpCodes.Ldarg_0);
            methodUidGetIL2.Emit(OpCodes.Ldfld, fieldUid);
            methodUidGetIL2.Emit(OpCodes.Ret);
            propertyName.SetGetMethod(methodUidGet2);



            // 添加构造函数
            ConstructorBuilder constructor = typeBuilder.DefineConstructor(MethodAttributes.Public, CallingConventions.Standard, Type.EmptyTypes);
            ILGenerator constructorIL = constructor.GetILGenerator();
            constructorIL.Emit(OpCodes.Ldarg_0);
            constructorIL.Emit(OpCodes.Call, typeof(object).GetConstructor(Type.EmptyTypes));
            constructorIL.Emit(OpCodes.Ldarg_0);
            constructorIL.Emit(OpCodes.Call, typeof(EngineObject.UID).GetMethod("GetUID", BindingFlags.Static | BindingFlags.Public));
            constructorIL.Emit(OpCodes.Stfld, fieldUid);
            constructorIL.Emit(OpCodes.Ret);

            // 创建 EngineObject 类型
            Type engineObjectType = typeBuilder.CreateType();


            // save dll
            assemblyBuilder.Save("CkfEngine.Core.dll");

            Console.WriteLine("Dynamic assembly saved.");
        }
    }
}
