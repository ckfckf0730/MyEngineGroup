using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal static class CommonFuction
    {
        public static byte[] StructToByteArray<T>(T value)
        {
            if (value == null)
            {
                throw new ArgumentNullException(nameof(value), "Value cannot be null");
            }

            Type type = value.GetType();

            if (!type.IsValueType)
            {
                throw new ArgumentException("Value must be a value type", nameof(value));
            }

            int size = Marshal.SizeOf(value);
            byte[] byteArray = new byte[size];

            IntPtr ptr = Marshal.AllocHGlobal(size);
            try
            {
                Marshal.StructureToPtr(value, ptr, true);
                Marshal.Copy(ptr, byteArray, 0, size);
            }
            finally
            {
                Marshal.FreeHGlobal(ptr);
            }

            return byteArray;
        }

        public static T ByteArrayToStructure<T>(byte[] bytes) where T : struct
        {
            // Alloc memory
            GCHandle handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            try
            {
                return (T)Marshal.PtrToStructure(handle.AddrOfPinnedObject(), typeof(T));
            }
            finally
            {
                handle.Free();
            }
        }

        public static object ByteArrayToObject(byte[] bytes, Type type)
        {
            // Alloc memory
            GCHandle handle = GCHandle.Alloc(bytes, GCHandleType.Pinned);
            try
            {
                return Marshal.PtrToStructure(handle.AddrOfPinnedObject(), type);
            }
            finally
            {
                handle.Free();
            }
        }

        public static string[] GetCurlyBracketsContents(string text)
        {
            string lastText = text;
            List<string> strings = new List<string>();

            while (true)
            {
                var index = lastText.IndexOf('{');
                if(index == -1)
                {
                    break;
                }

                int rightIndex = FindRightCurlyBracketIndex(lastText, index);

                strings.Add(lastText.Substring(0, index));
                strings.Add(lastText.Substring(index + 1, rightIndex - index));
                lastText = lastText.Substring(rightIndex + 1);
            }

            return strings.ToArray();
        }

        private static int FindRightCurlyBracketIndex(string text, int startIndex)
        {
            int sumLeft = 1;
            for(int i = startIndex + 1; i< text.Length; i++)
            {
                if (text[i] == '{')
                {
                    sumLeft++;
                }
                else if (text[i] == '}')
                {
                    sumLeft--;
                    if(sumLeft == 0)
                    {
                        return i;
                    }

                }
            }

            throw new Exception("Can't find right curly bracket!");
        }


        public static List<T> DeepCopy<T>(List<T> list)
        {
            using (MemoryStream ms = new MemoryStream())
            {
                BinaryFormatter formatter = new BinaryFormatter();
                formatter.Serialize(ms, list);
                ms.Position = 0;
                return (List<T>)formatter.Deserialize(ms);
            }
        }
    }
}
