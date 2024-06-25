using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal static class CommonFuction
    {
        public static byte[] StructToByteArray<T>(T obj) where T : struct
        {
            int size = Marshal.SizeOf(obj);
            byte[] arr = new byte[size];
            IntPtr ptr = Marshal.AllocHGlobal(size);

            try
            {
                Marshal.StructureToPtr(obj, ptr, true);
                Marshal.Copy(ptr, arr, 0, size);
            }
            finally
            {
                Marshal.FreeHGlobal(ptr);
            }

            return arr;
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
    }
}
