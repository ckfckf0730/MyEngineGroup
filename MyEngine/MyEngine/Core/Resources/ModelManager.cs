using System;
using System.Collections;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using static CkfEngine.Core.PMDModel;

namespace CkfEngine.Core
{
    internal static class ModelManager
    {



        private static Dictionary<string, PMDModel> s_modelTable = new Dictionary<string, PMDModel>();

        internal static void LoadPMD(string path)
        {
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
                        return;
                    }

                    PMDModel pmdModel = new PMDModel();

                    pmdModel.m_header.version = reader.ReadSingle();
                    pmdModel.m_header.model_name = jis.GetString(reader.ReadBytes(20)); // 读取固定长度的字符数组并转换为字符串
                    pmdModel.m_header.comment = jis.GetString(reader.ReadBytes(256));  // 读取字符串

                    const uint pmdvertex_size_inFIle = 38;
                    const uint pmdvertex_size_inGPU = 40;

                    pmdModel.m_vertNum = reader.ReadUInt32();
                    pmdModel.m_vertices = new List<byte>();
                    pmdModel.m_vertices.AddRange(reader.ReadBytes((int)(pmdModel.m_vertNum * pmdvertex_size_inFIle))) ;
                    //std::vector < unsigned char> readFileVertices(m_vertNum * pmdvertex_size_inFIle);
                    //fread(readFileVertices.data(), readFileVertices.size(), 1, fp); //next PMDvertex data

                    //std::vector < unsigned short> indices;
                    //fread(&m_indicesNum, sizeof(m_indicesNum), 1, fp);  //next indices number
                    //indices.resize(m_indicesNum);
                    //size_t indicesAllData_size = m_indicesNum * sizeof(indices[0]);
                    //fread(indices.data(), indicesAllData_size, 1, fp); //next indices data

                    //unsigned int materialNum;
                    //fread(&materialNum, sizeof(materialNum), 1, fp);

                    //std::vector<PMDMaterial> pmdMaterials(materialNum);
                    //fread(pmdMaterials.data(), pmdMaterials.size() * sizeof(PMDMaterial), 1, fp);
                }
            }
            catch(Exception e)
            {
                Console.WriteLine(e.Message);
            }
            
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




        public struct PMDHeader
        {
            public float version;
            public string model_name;
            public string comment;
        };
    }


    public class Model
    {
        internal uint m_vertNum;

        internal List<byte> m_vertices;
    }
}
