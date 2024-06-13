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
        internal static uint materialId = 0;

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

        internal static bool SetMaterials(string pipelineName, Model model, out uint matId)
        {
            matId = materialId++;
            return D3DAPICall.SetMaterials(matId, pipelineName,(uint) model.m_materials.Count,
                model.m_materials.Select(item => item.diffuse).ToArray(),
                model.m_materials.Select(item => item.alpha).ToArray(),
                model.m_materials.Select(item => item.specularity).ToArray(),
                model.m_materials.Select(item => item.specular).ToArray(),
                model.m_materials.Select(item => item.ambient).ToArray(),
                model.m_materials.Select(item => item.edgeFlg).ToArray(),
                model.m_materials.Select(item => item.toonPath).ToArray(),
                model.m_materials.Select(item => item.indicesNum).ToArray(),
                model.m_materials.Select(item => item.texFilePath).ToArray()) == 1;
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
