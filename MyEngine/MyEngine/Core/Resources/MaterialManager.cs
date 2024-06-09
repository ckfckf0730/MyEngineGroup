using System;
using System.Collections.Generic;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal static class MaterialManager
    {
        public static void GetPMDTexturePaths(List<StandardMaterial> matList)
        {
            //GetToonPath(matList[0].toonIdx);
        }

        public static string GetToonPath(byte toonIdx)
        {
            string toonFilePath = "Assets/toon/";
            toonIdx++;
            string idxString = toonIdx.ToString();
            if (toonIdx / 10 == 0)
            {
                idxString = "0" + idxString;
            }
            string toonFileName = "toon" + idxString + ".bmp";
            toonFilePath += toonFileName;
            return toonFilePath;
        }

    }

    internal class StandardMaterial : MaterialBase
    {
        public Vector3 diffuse;
        public float alpha;
        public float specularity;
        public Vector3 specular;
        public Vector3 ambient;
        public string toonPath;
        public byte edgeFlg;
        public uint indicesNum;
        public string texFilePath;  //20bytes
    }

    internal class MaterialBase
    {
        public Shader shader;

        public bool isShared = true;
    }

    

}
