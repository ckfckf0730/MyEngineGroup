using System;
using System.Collections.Generic;
using System.Globalization;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{

    internal static class ShaderDataTypeManager
    {

        private static Dictionary<string, ShaderDataType> Table;

        internal static ShaderDataType GetTypeByString(string typeName)
        {
            Table.TryGetValue(typeName, out var type);
            return type;
        }

        internal static void Init()
        {
            Table = new Dictionary<string, ShaderDataType>();

            //-------------------- COLOR --------------------
            ShaderDataType type = new ShaderDataType();
            type.name = "COLOR";
            type.type = typeof(Vector4);
            type.arrLen = 4;
            type.limitedMin = 0.0f;
            type.limitedMax = 1.0f;
            type.StringToValue = (str) =>
            {
                if(string.IsNullOrEmpty(str))
                {
                    return Vector4.Zero;
                }

                str = str.Trim('<', '>');

                string[] values = str.Split(',');

                if (values.Length != 4)
                {
                    throw new FormatException("Invalid vector format");
                }

                float x = float.Parse(values[0], CultureInfo.InvariantCulture);
                float y = float.Parse(values[1], CultureInfo.InvariantCulture);
                float z = float.Parse(values[2], CultureInfo.InvariantCulture);
                float w = float.Parse(values[3], CultureInfo.InvariantCulture);

                return new Vector4(x, y, z,w);
            };

            Table.Add(type.name, type);
        }

        internal static ValueType GetValueByString(string typeName, string data)
        {
            if(Table.TryGetValue(typeName, out var type))
            {
                return type.StringToValue(data);
            }

            Console.WriteLine( "Can't find shader data type: " + typeName);
            return 0;
        }

        internal static byte[] GetBytesByString(string typeName, string data)
        {
            var value = GetValueByString(typeName, data);

            return CommonFuction.StructToByteArray(value);
        }
    }


    internal struct ShaderDataType
    {
        public string name;
        public Type type;
        public uint arrLen;
        public float limitedMin;
        public float limitedMax;

        public Func<string, ValueType> StringToValue;
    }
}
