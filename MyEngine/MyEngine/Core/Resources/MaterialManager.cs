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

        public static void CreateCustomizedResource(StandardMaterial material)
        {
            uint firstNum = 3;  //0,1,2 are used for  camera,transform, basic material.
            foreach (var rootParameter in material.shader.rootParameters)
            {
                D3DAPICall.CreateCustomizedResource(material.materialId, rootParameter.name, 12, firstNum);
                var data = ShaderDataTypeManager.GetBytesByString(rootParameter.dataType, rootParameter.defaultValue);
                D3DAPICall.SetCustomizedResourceValue(material.materialId, rootParameter.name, data);
                firstNum++;
            }
            D3DAPICall.CreateCustomizedDescriptors(material.materialId, material.shader.m_name);
        }

        public static void SetCustomizedResourceValue(StandardMaterial material, string paramName , object value)
        {
            var name = material.shader.rootParameters.Find( item => item.name == paramName ).name;
            var data = CommonFuction.StructToByteArray(value);
            D3DAPICall.SetCustomizedResourceValue(material.materialId, name, data);
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

        internal static bool SetMaterials( List<StandardMaterial> materials, out uint[] IDs)
        {
            List <StandardMaterial> setList = new List < StandardMaterial >();
            foreach(var item in materials)
            {
                if (PrepareSet(item))
                {
                    setList.Add(item);
                }
            }

            IDs = setList.Select(item => item.materialId).ToArray();
            return D3DAPICall.SetMaterials(
               IDs,
                (uint)setList.Count,
                setList.Select(item => item.shader.m_name).ToArray(),
                setList.Select(item => item.diffuse).ToArray(),
                setList.Select(item => item.alpha).ToArray(),
                setList.Select(item => item.specularity).ToArray(),
                setList.Select(item => item.specular).ToArray(),
                setList.Select(item => item.ambient).ToArray(),
                setList.Select(item => item.edgeFlg).ToArray(),
                setList.Select(item => item.toonPath).ToArray(),
                setList.Select(item => item.texFilePath).ToArray()) == 1;
        }

        private static bool PrepareSet(StandardMaterial material)
        {
            if(material.isSetted)
            {
                return false;
            }
            material.materialId = materialId++;
            material.isSetted = true;

            return true;
        }

        internal static List<StandardMaterial> InstantiateMaterials(List<StandardMaterial> materials)
        {
            var list = CommonFuction.DeepCopy(materials);
            foreach(var item in list)
            {
                item.isShared = false;
            }
            return list;
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
        //public uint indicesNum;
        public string texFilePath;  //20bytes
    }

    internal class MaterialBase
    {
        public Shader shader;
        public uint materialId;

        public bool isSetted = false;
        public bool isShared = true;
    }

    

}
