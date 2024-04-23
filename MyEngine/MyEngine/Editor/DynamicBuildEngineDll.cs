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
using System.Runtime.InteropServices;
using System.Security.AccessControl;

namespace CkfEngine.Editor
{
    internal static class DynamicBuildEngineDll
    {
        private static TypeInfo EntityType;
        internal static void BuildCore(string path)
        {
            AssemblyName assemblyName = new AssemblyName("CkfEngine");

            AssemblyBuilder assemblyBuilder = AssemblyBuilder.DefineDynamicAssembly(assemblyName, AssemblyBuilderAccess.Save);

            // create module
            ModuleBuilder moduleBuilder = assemblyBuilder.DefineDynamicModule("CkfEngine.Core", "CkfEngine.dll");


            BuildType<EngineObject>(moduleBuilder);
            BuildType<Entity>(moduleBuilder);
            BuildType<Component>(moduleBuilder);
            BuildType<Transform>(moduleBuilder);
            BuildType<Behaviour>(moduleBuilder);




            assemblyBuilder.Save("CkfEngine.dll");
            System.IO.File.Copy("CkfEngine.dll", path + "/CkfEngine.dll");

            Console.WriteLine("Dynamic assembly saved.");
        }


        internal static void BuildType<T>(ModuleBuilder moduleBuilder)
        {
            // create class 
            Type type = typeof(T);
            string typeName = type.FullName;
            bool isSerialzable = type.GetCustomAttribute<SerializableAttribute>() != null;
            TypeAttributes typeAttributes = (type.IsPublic ? TypeAttributes.Public : TypeAttributes.NotPublic) |
                (isSerialzable ? TypeAttributes.Serializable : 0) |
                (type.IsAbstract ? TypeAttributes.Abstract : 0);

            var _baseType = type.BaseType;

            TypeBuilder typeBuilder = moduleBuilder.DefineType(typeName, typeAttributes, _baseType);

            // add attribute
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
            var fieldInfos = type.GetFields(BindingFlags.Instance | BindingFlags.Public)
                .Where(m => m.DeclaringType == type);  //don't add basic class field
            foreach ( var fieldInfo in fieldInfos)
            {
                FieldBuilder fieldBuilder = typeBuilder.DefineField(fieldInfo.Name, fieldInfo.FieldType, fieldInfo.Attributes);
            }

            // add Property
            var propertyInfos = type.GetProperties(BindingFlags.Instance | BindingFlags.Public).
                Where(m => m.DeclaringType == type);        
            foreach (var propertyInfo in propertyInfos)
            {
                var propertyType = propertyInfo.PropertyType;
                PropertyBuilder propertyBuilder = typeBuilder.DefineProperty(propertyInfo.Name, PropertyAttributes.None, propertyType, null);
  
                //get
                if(propertyInfo.CanRead)
                {
                    MethodBuilder getMethodBuilder = typeBuilder.DefineMethod("get_" + propertyInfo.Name, 
                        MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.HideBySig, propertyType, Type.EmptyTypes);
                    ILGenerator methodGetIL = getMethodBuilder.GetILGenerator();
                    //△△△△△△△△△△△ The Final App Run by Engine project,  game project just script files  △△△△△△△△△△△△
                    //△△△△△△△△△△△ The dll file just provide interface to game project for easy edit △△△△△△△△△△△△
                    //△△△△△△△△△△△ So will not realize the function body △△△△△△△△△△△△
                    methodGetIL.ThrowException(typeof(Exception));
                    propertyBuilder.SetGetMethod(getMethodBuilder);
                }
                if(propertyInfo.CanWrite)
                {
                    MethodBuilder setMethodBuilder = typeBuilder.DefineMethod("set_" + propertyInfo.Name, 
                        MethodAttributes.Public | MethodAttributes.SpecialName | MethodAttributes.HideBySig, null, new Type[] { propertyType });
                    ILGenerator setIl = setMethodBuilder.GetILGenerator();
                    setIl.ThrowException(typeof(Exception));

                    ILGenerator methodGetIL = setMethodBuilder.GetILGenerator();
                    methodGetIL.ThrowException(typeof(Exception));
                    propertyBuilder.SetSetMethod(setMethodBuilder);
                }
            }

            // add Method 
            var methodInfos = type.GetMethods(
                BindingFlags.Public| BindingFlags.Instance |BindingFlags.NonPublic). 
                Where(m => m.DeclaringType == type).
                Where(m => !m.Name.StartsWith("get_") && !m.Name.StartsWith("set_")).ToArray();
            foreach (var methodInfo in methodInfos)
            {
                if (methodInfo.IsPrivate)
                {
                    continue;
                }

                var paramenterInfos = methodInfo.GetParameters();
                Type[] types = new Type[paramenterInfos.Length];
                for (int i = 0; i < types.Length; i++)
                {
                    types[i] = paramenterInfos[i].ParameterType;
                }

                MethodAttributes methodAttributes = 0;
                methodAttributes |= methodInfo.IsFamily ? MethodAttributes.Family : MethodAttributes.Public;
                methodAttributes |= methodInfo.IsVirtual ? MethodAttributes.Virtual : 0;
                MethodBuilder methodBuilder = typeBuilder.DefineMethod(methodInfo.Name, methodAttributes,
                    methodInfo.ReturnType, types);

                //deal with generic info
                Type[] genericArguments = methodInfo.GetGenericArguments();
                GenericTypeParameterBuilder[] genericTypes = new GenericTypeParameterBuilder[genericArguments.Length];
                for (int i = 0; i < genericArguments.Length; i++)
                {
                    genericTypes[i] = methodBuilder.DefineGenericParameters(genericArguments[i].Name)[i];
                    genericTypes[i].SetBaseTypeConstraint(genericArguments[i].BaseType);
                }

                ILGenerator ilGenerator = methodBuilder.GetILGenerator();
                ilGenerator.ThrowException(typeof(Exception));
            }

            // add static Method 
            var methodinfos2 = type.GetMethods(BindingFlags.Public | BindingFlags.Static).
                Where(m => m.DeclaringType == type).ToArray();
            foreach (var methodInfo in methodinfos2)
            {
                var paramenterInfos = methodInfo.GetParameters();
                Type[] types = new Type[paramenterInfos.Length];
                for (int i = 0; i < types.Length; i++)
                {
                    types[i] = paramenterInfos[i].ParameterType;
                }
                MethodBuilder methodBuilder = typeBuilder.DefineMethod(methodInfo.Name, MethodAttributes.Public | MethodAttributes.Static,
                    methodInfo.ReturnType, types);
                ILGenerator ilGenerator = methodBuilder.GetILGenerator();
                ilGenerator.ThrowException(typeof(Exception));
            }

            // add constructors
            var constructorInfos = type.GetConstructors(BindingFlags.Public | BindingFlags.Instance).
                Where(m => m.DeclaringType == type);
            foreach(var constructorInfo in constructorInfos)
            {
                var paramenterInfos = constructorInfo.GetParameters();
                Type[] types = new Type[paramenterInfos.Length];
                for (int i = 0; i < types.Length; i++)
                {
                    types[i] = paramenterInfos[i].ParameterType;
                }
                ConstructorBuilder constructorBuilder = typeBuilder.DefineConstructor(MethodAttributes.Public,
                    CallingConventions.Standard, types);
                ILGenerator constructorIL = constructorBuilder.GetILGenerator();
                constructorIL.ThrowException(typeof(Exception));
            }

            // Create type
            typeBuilder.CreateType();

        }
    }
}
