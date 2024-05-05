using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Runtime.InteropServices.ComTypes;
using System.Text;
using System.Threading.Tasks;
using static CkfEngine.Core.PMDModel;

namespace CkfEngine.Core
{
    internal static class ModelManager
    {



        private static Dictionary<string, PMDModel> s_modelTable = new Dictionary<string, PMDModel>();

        internal static bool LoadPMDFile(string path, out PMDModel pmdModel)
        {
            if (s_modelTable.TryGetValue(path, out pmdModel))
            {
                return true;
            }

            try
            {
                var jis = Encoding.GetEncoding("shift_jis");
                using (FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read))
                using (BinaryReader reader = new BinaryReader(fs, jis))
                {
                    string signature = jis.GetString(reader.ReadBytes(3));
                    if (signature.ToLower() != "pmd")
                    {
                        Console.WriteLine(path + " is not PMD file.");
                        return false;
                    }

                    pmdModel = new PMDModel();

                    //--------------------header-------------------
                    pmdModel.m_header.version = reader.ReadSingle();
                    pmdModel.m_header.model_name = jis.GetString(reader.ReadBytes(20));
                    pmdModel.m_header.comment = jis.GetString(reader.ReadBytes(256));


                    //--------------------vertex and index--------------------
                    const uint pmdvertex_size_inFIle = 38;

                    pmdModel.m_vertextCount = reader.ReadUInt32();
                    pmdModel.m_vertices = new List<byte>();
                    pmdModel.m_vertices.AddRange(reader.ReadBytes((int)(pmdModel.m_vertextCount * pmdvertex_size_inFIle)));

                    pmdModel.m_indexCount = reader.ReadUInt32();
                    var byteArr = reader.ReadBytes((int)pmdModel.m_indexCount * sizeof(ushort));
                    pmdModel.m_indices = new List<ushort>();
                    for (int i = 0; i < pmdModel.m_indexCount; i++)
                    {
                        pmdModel.m_indices.Add(BitConverter.ToUInt16(byteArr, i * 2));
                    }


                    //--------------------Material--------------------
                    pmdModel.m_materialCount = reader.ReadUInt32();

                    pmdModel.m_materials = new List<StandardMaterial>();

                    for (int i = 0; i < pmdModel.m_materialCount; i++)
                    {
                        StandardMaterial material = new StandardMaterial();

                        material.diffuse.X = reader.ReadSingle();
                        material.diffuse.Y = reader.ReadSingle();
                        material.diffuse.Z = reader.ReadSingle();
                        material.alpha = reader.ReadSingle();
                        material.specularity = reader.ReadSingle();
                        material.specular.X = reader.ReadSingle();
                        material.specular.Y = reader.ReadSingle();
                        material.specular.Z = reader.ReadSingle();
                        material.ambient.X = reader.ReadSingle();
                        material.ambient.Y = reader.ReadSingle();
                        material.ambient.Z = reader.ReadSingle();
                        material.toonIdx = reader.ReadByte();
                        material.edgeFlg = reader.ReadByte();
                        material.indicesNum = reader.ReadUInt32();
                        material.texFilePath = jis.GetString(reader.ReadBytes(20));

                        pmdModel.m_materials.Add(material);
                    }

                    //--------------------bone data--------------------
                    pmdModel.m_boneCount = reader.ReadUInt16();

                    pmdModel.m_bones = new List<PMDBone>();
                    for (int i = 0; i < pmdModel.m_boneCount; i++)
                    {
                        PMDBone pmdBone = new PMDBone();

                        pmdBone.boneName = jis.GetString(reader.ReadBytes(20));
                        pmdBone.parentNo = reader.ReadUInt16();
                        pmdBone.nextNo = reader.ReadUInt16();
                        pmdBone.type = reader.ReadByte();
                        pmdBone.ikBoneNo = reader.ReadUInt16();
                        pmdBone.pos.X = reader.ReadSingle();
                        pmdBone.pos.Y = reader.ReadSingle();
                        pmdBone.pos.Z = reader.ReadSingle();

                        pmdModel.m_bones.Add(pmdBone);
                    }

                    //--------------------ik data--------------------
                    pmdModel.m_ikCount = reader.ReadUInt16();
                    pmdModel.m_iks = new List<PMDIK>();
                    for (int i = 0; i < pmdModel.m_ikCount; i++)
                    {
                        PMDIK pmdIk = new PMDIK();
                        pmdIk.boneIdx = reader.ReadUInt16();
                        pmdIk.targetIdx = reader.ReadUInt16();
                        pmdIk.chainLen = reader.ReadByte();
                        pmdIk.iterations = reader.ReadUInt16();
                        pmdIk.limit = reader.ReadSingle();

                        pmdIk.nodeIdxes = new List<ushort>();
                        for (int j = 0; j < pmdIk.chainLen; j++)
                        {
                            pmdIk.nodeIdxes.Add(reader.ReadUInt16());
                        }


                        pmdModel.m_iks.Add(pmdIk);
                    }

                    s_modelTable.Add(path, pmdModel);
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                return false;
            }
            return true;
        }

        internal static bool SetPMDVertices(string path, PMDModel pmdModel)
        {
            return D3DAPICall.SetPMDVertices(path, pmdModel.m_vertextCount,
                pmdModel.m_vertices.ToArray(), pmdModel.m_indexCount, pmdModel.m_indices.ToArray()) == 1;
            
        }

        internal static bool SetPMDMaterials(string path, PMDModel pmdModel)
        {
            return D3DAPICall.SetPMDMaterials(path, pmdModel.m_materialCount, pmdModel.m_materials.Select(item => item.diffuse).ToArray(),
                pmdModel.m_materials.Select(item => item.alpha).ToArray(),
                pmdModel.m_materials.Select(item => item.specularity).ToArray(),
                pmdModel.m_materials.Select(item => item.specular).ToArray(),
                pmdModel.m_materials.Select(item => item.ambient).ToArray(),
                pmdModel.m_materials.Select(item => item.edgeFlg).ToArray(),
                pmdModel.m_materials.Select(item => item.toonIdx).ToArray(),
                pmdModel.m_materials.Select(item => item.indicesNum).ToArray(),
                pmdModel.m_materials.Select(item => item.texFilePath).ToArray()) == 1;
        }

        internal static bool SetPMDBoneIk(string path, PMDModel pmdModel)
        {
            var nodeIdxes = pmdModel.m_iks.Select(item => item.nodeIdxes.ToArray()).ToArray();
            ushort[] flattenedArray = nodeIdxes.SelectMany(row => row).ToArray(); 
            return D3DAPICall.SetPMDBoneIk(path,
                pmdModel.m_boneCount,
                pmdModel.m_ikCount,
                pmdModel.m_bones.Select(item => item.boneName).ToArray(),
                pmdModel.m_bones.Select(item => item.parentNo).ToArray(),
                pmdModel.m_bones.Select(item => item.nextNo).ToArray(),
                pmdModel.m_bones.Select(item => item.type).ToArray(),
                pmdModel.m_bones.Select(item => item.ikBoneNo).ToArray(),
                pmdModel.m_bones.Select(item => item.pos).ToArray(),
                pmdModel.m_iks.Select(item => item.boneIdx).ToArray(),
                pmdModel.m_iks.Select(item => item.targetIdx).ToArray(),
                pmdModel.m_iks.Select(item => item.iterations).ToArray(),
                pmdModel.m_iks.Select(item => item.limit).ToArray(),
                pmdModel.m_iks.Select(item => item.chainLen).ToArray(),
                flattenedArray) == 1;
        }

        internal static bool InstantiatePMDModel(ulong uid, string path)
        {
            return D3DAPICall.InstantiatePMDModel(uid, path) == 1;
        }


        private static T ByteArrayToStructure<T>(byte[] bytes) where T : struct
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

    }


    public class PMDModel : Model
    {
        internal PMDHeader m_header;

        internal ushort m_boneCount;
        internal List<PMDBone> m_bones;
        internal ushort m_ikCount;
        internal List<PMDIK> m_iks;

        public struct PMDHeader
        {
            public float version;
            public string model_name;
            public string comment;
        };
    }


    public class Model
    {
        internal uint m_vertextCount;
        internal uint m_indexCount;
        internal uint m_materialCount;

        internal List<byte> m_vertices;
        internal List<ushort> m_indices;

        internal List<StandardMaterial> m_materials;


    }




    struct PMDBone
    {
        public string boneName;  //20bytes
        public ushort parentNo;
        public ushort nextNo;
        public byte type;
        public ushort ikBoneNo;
        public Vector3 pos;
    }

    struct PMDIK
    {
        public ushort boneIdx;
        public ushort targetIdx;
        public byte chainLen;  //the number of node between ...
        public ushort iterations;
        public float limit;
        public List<ushort> nodeIdxes;
    };

}
