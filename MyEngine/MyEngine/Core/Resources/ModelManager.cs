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

namespace CkfEngine.Core
{
    internal static class ModelManager
    {

        private static Dictionary<string, PMDModel> s_modelTable = new Dictionary<string, PMDModel>();

        private static Dictionary<string, VMDAnimation> s_animationTable = new Dictionary<string, VMDAnimation>();

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
                        material.texFilePath = jis.GetString(reader.ReadBytes(20)).Split('\0')[0];

                        pmdModel.m_materials.Add(material);
                    }

                    //--------------------bone data--------------------
                    pmdModel.m_boneCount = reader.ReadUInt16();

                    pmdModel.m_bones = new List<PMDBone>();
                    for (int i = 0; i < pmdModel.m_boneCount; i++)
                    {
                        PMDBone pmdBone = new PMDBone();

                        pmdBone.boneName = jis.GetString(reader.ReadBytes(20)).Split('\0')[0];
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

                    pmdModel.SetBone();
                }
            }
            catch (Exception e)
            {
                Console.WriteLine(e.Message);
                return false;
            }
            return true;
        }

        internal static bool LoadVMDFile(string path, out VMDAnimation vmdAnime)
        {
            if (s_animationTable.TryGetValue(path, out vmdAnime))
            {
                return true;
            }

            try
            {
                var jis = Encoding.GetEncoding("shift_jis");
                using (FileStream fs = new FileStream(path, FileMode.Open, FileAccess.Read))
                using (BinaryReader reader = new BinaryReader(fs, jis))
                {
                    reader.BaseStream.Seek(50, SeekOrigin.Begin);

                    vmdAnime = new VMDAnimation();

                    //-----------motion data----------------
                    uint motionDataNum = 0;
                    motionDataNum = reader.ReadUInt32();
                    List<VMDMotion> vmdMotionData = new List<VMDMotion>();
                    for (int i = 0; i < motionDataNum; i++)
                    {
                        VMDMotion vmdMotion = new VMDMotion();
                        vmdMotion.boneName = jis.GetString(reader.ReadBytes(15)).Split('\0')[0];
                        vmdMotion.frameNo = reader.ReadUInt32();
                        vmdMotion.location.X = reader.ReadSingle();
                        vmdMotion.location.Y = reader.ReadSingle();
                        vmdMotion.location.Z = reader.ReadSingle();
                        vmdMotion.quaternion.X = reader.ReadSingle();
                        vmdMotion.quaternion.Y = reader.ReadSingle();
                        vmdMotion.quaternion.Z = reader.ReadSingle();
                        vmdMotion.quaternion.W = reader.ReadSingle();
                        vmdMotion.bezier = reader.ReadBytes(64);

                        vmdMotionData.Add(vmdMotion);
                    }

                    //-----------morph data----------------
                    uint morphCount = 0;
                    morphCount = reader.ReadUInt32();
                    vmdAnime.m_morphs = new VMDMorph[morphCount];
                    for (int i =0;i< morphCount;i++)
                    {
                        vmdAnime.m_morphs[i].name = jis.GetString(reader.ReadBytes(15)).Split('\0')[0];
                        vmdAnime.m_morphs[i].frameNo = reader.ReadUInt32();
                        vmdAnime.m_morphs[i].wight = reader.ReadSingle();
                    }

                    //-----------camera animation data----------------
                    uint vmdCameraCount = 0;
                    vmdCameraCount = reader.ReadUInt32();
                    vmdAnime.m_cameraData = new VMDCamera[vmdCameraCount];
                    for (int i = 0; i < vmdCameraCount; i++)
                    {
                        vmdAnime.m_cameraData[i].frameNo = reader.ReadUInt32();
                        vmdAnime.m_cameraData[i].distance = reader.ReadSingle();
                        vmdAnime.m_cameraData[i].pos.X = reader.ReadSingle();
                        vmdAnime.m_cameraData[i].pos.Y = reader.ReadSingle();
                        vmdAnime.m_cameraData[i].pos.Z = reader.ReadSingle();
                        vmdAnime.m_cameraData[i].eulerAngle.X = reader.ReadSingle();
                        vmdAnime.m_cameraData[i].eulerAngle.Y = reader.ReadSingle();
                        vmdAnime.m_cameraData[i].eulerAngle.Z = reader.ReadSingle();
                        vmdAnime.m_cameraData[i].Interpolation = reader.ReadBytes(24);
                        vmdAnime.m_cameraData[i].fov = reader.ReadUInt32();
                        vmdAnime.m_cameraData[i].persFlg = reader.ReadByte();

                    }

                    //-----------light data----------------
                    uint vmdLightCount = 0;
                    vmdLightCount = reader.ReadUInt32();
                    vmdAnime.m_lights = new VMDLight[vmdLightCount];
                    for (int i = 0; i < vmdLightCount; i++)
                    {
                        vmdAnime.m_lights[i].frameNo = reader.ReadUInt32();
                        vmdAnime.m_lights[i].rgb.X = reader.ReadSingle();
                        vmdAnime.m_lights[i].rgb.Y = reader.ReadSingle();
                        vmdAnime.m_lights[i].rgb.Z = reader.ReadSingle();
                        vmdAnime.m_lights[i].vec.X = reader.ReadSingle();
                        vmdAnime.m_lights[i].vec.Y = reader.ReadSingle();
                        vmdAnime.m_lights[i].vec.Z = reader.ReadSingle();
                    }

                    //-----------self shadow data----------------
                    uint selfShadowCount = 0;
                    selfShadowCount = reader.ReadUInt32();
                    vmdAnime.m_selfShadowData = new VMDSelfShadow[selfShadowCount];
                    for (int i = 0; i < selfShadowCount; i++)
                    {
                        vmdAnime.m_selfShadowData[i].frameNo = reader.ReadUInt32();
                        vmdAnime.m_selfShadowData[i].mode = reader.ReadByte();
                        vmdAnime.m_selfShadowData[i].distance = reader.ReadSingle();
                    }

                    //-----------IK enable data----------------
                    uint ikSwitchCount = 0;
                    ikSwitchCount = reader.ReadUInt32();
                    vmdAnime.m_ikEnableData = new VMDIKEnable[ikSwitchCount];
                    for (int i = 0; i < ikSwitchCount;i ++)
                    {
                        vmdAnime.m_ikEnableData[i].ikEnableTable = new Dictionary<string, bool>();

                        vmdAnime.m_ikEnableData[i].frameNo = reader.ReadUInt32();
                        byte visibleFlg = reader.ReadByte();
                        uint ikBoneCount = reader.ReadUInt32();
                        for(int j =0; j< ikBoneCount;j++)
                        {
                            string ikBoneName = jis.GetString(reader.ReadBytes(20)).Split('\0')[0];
                            bool flg = reader.ReadByte() != 0;
                            vmdAnime.m_ikEnableData[i].ikEnableTable.Add(ikBoneName, flg);
                        }
                    }

                    s_animationTable.Add(path, vmdAnime);

                    foreach (var vmdMotion in vmdMotionData)
                    {
                        Quaternion quaternion = vmdMotion.quaternion;
                        List<KeyFrame> keyFramelist;
                        bool isGet = vmdAnime.m_motionData.TryGetValue(vmdMotion.boneName, out keyFramelist);
                        if(!isGet)
                        {
                            keyFramelist = new List<KeyFrame> ();
                            vmdAnime.m_motionData.Add(vmdMotion.boneName, keyFramelist);
                        }

                        keyFramelist.Add(new
                            KeyFrame() { frameNo = vmdMotion.frameNo, quaternion = quaternion, offset = vmdMotion.location,
                            p1 = new Vector2() { X = (float)vmdMotion.bezier[3] / 127.0f, Y = (float)vmdMotion.bezier[7] / 127.0f },
                            p2 = new Vector2() { X = (float)vmdMotion.bezier[11] / 127.0f, Y = (float)vmdMotion.bezier[15] / 127.0f } });

                        vmdAnime.m_duration = Math.Max(vmdAnime.m_duration, vmdMotion.frameNo);
                    }

                    foreach (var motion in vmdAnime.m_motionData)
                    {
                        motion.Value.Sort((lval, rval) => lval.frameNo.CompareTo(rval.frameNo));
                        ;
                    }

                    vmdAnime.m_fileName = path;
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

        internal static bool InstantiatePMDModel(ulong uid, string path,int boneSize)
        {
            return D3DAPICall.InstantiatePMDModel(uid, path, boneSize) == 1;
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


        internal Dictionary<string, BoneNode> m_boneNodeTable;
        internal string[] m_boneNameArr;
        internal BoneNode[] m_boneNodeAddressArr;
        internal string m_rootNodeStr;
        internal List<uint> m_kneeIdxes;

        public struct PMDHeader
        {
            public float version;
            public string model_name;
            public string comment;
        };


        internal void SetBone()
        {
            m_boneNameArr = new string[m_bones.Count];
            m_boneNodeAddressArr = new BoneNode[m_bones.Count];

            if (m_bones.Count > 0)
            {
                m_rootNodeStr = m_bones[0].boneName;
            }

            m_kneeIdxes= new List<uint>();
            m_boneNodeTable = new Dictionary<string, BoneNode>();
            for (int i = 0; i < m_bones.Count; i++)
            {
                BoneNode node = new BoneNode();
                m_boneNodeTable.Add(m_bones[i].boneName, node);
                node.boneIdx = (uint)i;
                node.startPos = m_bones[i].pos;

                m_boneNameArr[i] = m_bones[i].boneName;
                m_boneNodeAddressArr[i] = node;

                string boneName = m_bones[i].boneName;
                //the knee data is a special one, a common bone data struct may be not like this
                if (boneName.IndexOf("ひざ") != -1)
		        {
                    m_kneeIdxes.Add((uint)i);
                }
            }

	        foreach(var pb in m_bones)
	        {
		        if (pb.parentNo >= m_bones.Count)
		        {
			        continue;
		        }

		        string parentName = m_boneNameArr[pb.parentNo];
                if(m_boneNodeTable[parentName].children == null)
                {
                    m_boneNodeTable[parentName].children = new List<BoneNode>();
                }

                m_boneNodeTable[parentName].children.Add(
			        m_boneNodeTable[pb.boneName]);
	        }
        }
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

    class BoneNode
    {
        public uint boneIdx;
        public uint boneType;
        public uint ikParentBone;
        public Vector3 startPos;
        public List<BoneNode> children;
    };

}
