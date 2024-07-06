using System;
using System.Collections.Generic;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal static class MaterialManager
    {
        internal static uint materialId = 0;

        internal static void GetPMDTexturePaths(List<StandardMaterial> matList)
        {
            //GetToonPath(matList[0].toonIdx);
        }

        internal static void CreateCustomizedResource(StandardMaterial material)
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

        internal static object GetCustomizedResourceValue(StandardMaterial material, string paramName, Type type)
        {
            IntPtr dataPtr;
            UInt16 size;

            D3DAPICall.GetCustomizedResourceValue(material.materialId, paramName, out dataPtr, out size);
            byte[] data = new byte[size];
            Marshal.Copy(dataPtr, data, 0, (int)size);

            return CommonFuction.ByteArrayToObject(data, type);
        }

        internal static void SetCustomizedResourceValue(StandardMaterial material, string paramName, object value)
        {
            var name = material.shader.rootParameters.Find(item => item.name == paramName).name;
            var data = CommonFuction.StructToByteArray(value);
            D3DAPICall.SetCustomizedResourceValue(material.materialId, name, data);
        }

        internal static string GetToonPath(byte toonIdx)
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

        /// <summary>
        /// Register Materials in d3d dll
        /// </summary>
        /// <param name="materials"></param>
        /// <returns></returns>
        internal static bool RegisterMaterials(List<StandardMaterial> materials)
        {
            foreach (var item in materials)
            {
                if (PrepareSet(item))
                {
                    if(D3DAPICall.SetMaterial(
                           item.materialId,
                           item.shader.m_name,
                           item.diffuse,
                           item.alpha,
                           item.specularity,
                           item.specular,
                           item.ambient,
                           item.edgeFlg,
                           item.toonPath,
                           item.texFilePath)    == 1 )
                    {
                        CreateCustomizedResource(item);
                    }
                }
            }


            return true;
        }

        internal static void SetMaterialValue(StandardMaterial material)
        {

            D3DAPICall.SetMaterialValue(
                           material.materialId,
                           material.shader.m_name,
                           material.diffuse,
                           material.alpha,
                           material.specularity,
                           material.specular,
                           material.ambient,
                           material.edgeFlg,
                           material.toonPath,
                           material.texFilePath);

        }


        internal static void SetInstanceMaterials(ulong UID, StandardMaterial[] matrials)
        {
            for (uint i = 0; i < matrials.Count(); i++)
            {
                //D3DAPICall.BindPipeline(UID, mat.shader.m_name);
                D3DAPICall.BindMaterialControl(UID, matrials[i].materialId, i);
            }
        }

        internal static void SetInstanceMaterial(ulong UID, StandardMaterial matrial, uint index)
        {
            D3DAPICall.BindMaterialControl(UID, matrial.materialId, index);
        }

        internal static void UnsetInstanceMaterial(ulong UID, StandardMaterial matrial, uint index)
        {
            D3DAPICall.UnBindMaterialControl(UID, matrial.materialId, index);
            
        }

        private static bool PrepareSet(StandardMaterial material)
        {
            if (material.isSetted)
            {
                return false;
            }
            material.materialId = materialId++;
            material.isSetted = true;

            return true;
        }

        internal static List<StandardMaterial> InstantiateMaterials(List<StandardMaterial> materials)
        {
            var list = new List<StandardMaterial>();

            foreach (var item in materials)
            {
                list.Add(item.Clone());
                list[list.Count - 1].isShared = false;
                list[list.Count - 1].isSetted = false;
            }
            return list;
        }

        internal static StandardMaterial InstantiateMaterial(StandardMaterial material)
        {
            var newMat = material.Clone();
            newMat.isShared = false;
            newMat.isSetted = false;
            return newMat;
        }

        internal static void ChangeMaterialShader(StandardMaterial material, Shader shader, ulong uid, uint index)
        {
            //the logic of check old material release
            //.........................

            material.shader = shader;
            MaterialManager.RegisterMaterials(new List<StandardMaterial>() { material });
            MaterialManager.SetInstanceMaterial(uid, material, index);
        }
    }

    [Serializable]
    public class StandardMaterial : MaterialBase
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

        public StandardMaterial Clone()
        {
            return (StandardMaterial)this.MemberwiseClone();
        }
    }

    [Serializable]
    public class MaterialBase
    {
        public Shader shader;
        public uint materialId;
        public string name;

        public bool isSetted = false;
        public bool isShared = true;
    }



}
