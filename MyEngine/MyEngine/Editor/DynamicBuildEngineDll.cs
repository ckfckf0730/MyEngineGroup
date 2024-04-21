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
using System.Runtime.Serialization;

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

            BuildType<EngineObject>(moduleBuilder);

            assemblyBuilder.Save("CkfEngine.Core.dll");



            Console.WriteLine("Dynamic assembly saved.");
        }


        internal static void BuildType<T>(ModuleBuilder moduleBuilder)
        {
            // create class 
            Type type = typeof(T);
            string typeName = type.FullName;
            bool isSerialzable = type.GetCustomAttribute<SerializableAttribute>() != null;
            TypeAttributes typeAttributes = (type.IsPublic ? TypeAttributes.Public : TypeAttributes.NotPublic) |
                (isSerialzable ? TypeAttributes.Serializable : 0);

            TypeBuilder typeBuilder = moduleBuilder.DefineType(typeName, typeAttributes);

            // add [Serializable] attribute
            var attributes = type.GetCustomAttributes();
            if(attributes.Count() != 0)
            {
                foreach(Attribute attribute in attributes) 
                {
                    //get attribute all fields info and their value 
                    var attFields = attribute.GetType().GetFields();
                    var attFieldValues = new object[attFields.Length];
                    for(int i =0;i< attFields.Length;i++)
                    {
                        attFieldValues[i] = attFields[i].GetValue(attribute);
                    }

                    //get attribute constructor info and it's parameters info
                    ConstructorInfo serializableCtor = attribute.GetType().GetConstructors()[0];
                    var attParameters = serializableCtor.GetParameters();
                    object[] attParametValues = new object[attParameters.Length];
                    for (int i = 0; i < attParameters.Length; i++)
                    {
                        //attParameters[i].ParameterType.IsValueType
                    }

                    CustomAttributeBuilder serializableBuilder = new CustomAttributeBuilder(
                        serializableCtor, attParametValues, attFields, attParametValues);

                    typeBuilder.SetCustomAttribute(serializableBuilder);
                }
            }

            // add instance field
            var fieldInfos = type.GetFields(BindingFlags.Instance | BindingFlags.Public);
            foreach( var fieldInfo in fieldInfos)
            {
                FieldBuilder fieldBuilder = typeBuilder.DefineField(fieldInfo.Name, fieldInfo.FieldType, fieldInfo.Attributes);


            }

            // add Property
            var propertyInfos = type.GetProperties();
            foreach (var propertyInfo in propertyInfos)
            {

                PropertyBuilder propertyUid = typeBuilder.DefineProperty(propertyInfo.Name, PropertyAttributes.None, typeof(string), null);
                MethodBuilder methodUidGet = typeBuilder.DefineMethod(propertyInfo.Name, MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.HideBySig, typeof(string), Type.EmptyTypes);
                ILGenerator methodUidGetIL = methodUidGet.GetILGenerator();
                //△△△△△△△△△△△ The Final App Run by Engine project,  game project just script files  △△△△△△△△△△△△
                //△△△△△△△△△△△ The dll file just provide interface to game project for easy edit △△△△△△△△△△△△
                //△△△△△△△△△△△ So will not realize the function body △△△△△△△△△△△△
                methodUidGetIL.ThrowException(typeof(Exception));
                propertyUid.SetGetMethod(methodUidGet);
            }

            // 添加构造函数
            ConstructorBuilder constructor = typeBuilder.DefineConstructor(MethodAttributes.Public, CallingConventions.Standard, Type.EmptyTypes);
            ILGenerator constructorIL = constructor.GetILGenerator();
            constructorIL.ThrowException(typeof(Exception));
            //constructorIL.Emit(OpCodes.Ldarg_0);
            //constructorIL.Emit(OpCodes.Call, typeof(object).GetConstructor(Type.EmptyTypes));
            //constructorIL.Emit(OpCodes.Ldarg_0);
            //constructorIL.Emit(OpCodes.Call, typeof(EngineObject.UID).GetMethod("GetUID", BindingFlags.Static | BindingFlags.Public));
            //constructorIL.Emit(OpCodes.Stfld, fieldUid);
            //constructorIL.Emit(OpCodes.Ret);


            // 创建 EngineObject 类型
            Type engineObjectType = typeBuilder.CreateType();


            Console.WriteLine("Dynamic assembly saved.");
        }
    }
}
