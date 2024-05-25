using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal static class RenderPipelineManager
    {
        public static void CreatePipeline(MaterialBase material,bool isBoneModel)
        {
            var matType = material.GetType();
        }

    }


    internal class Shader
    {
        internal static Shader BasicBoneShader;
        internal static Shader BasicNoBoneShader;
        internal static Dictionary<string,Shader> ShaderTable;

        internal static void InitBasicShader()
        {
            ShaderTable = new Dictionary<string, Shader>();
            string boneName = "BasicBoneShader";
            string noBoneName = "BasicNoBoneShader";

            string vsCode = File.ReadAllText("BasicVertexShader.hlsl");
            string psCode = File.ReadAllText("BasicPixelShader.hlsl");
            BasicBoneShader = new Shader(boneName, vsCode, psCode, "BasicVS", "BasicPS",true);

            vsCode = File.ReadAllText("NoBoneVertexShader.hlsl");
            BasicNoBoneShader = new Shader(noBoneName, vsCode, psCode, "BasicVS", "BasicPS", false);


            ShaderTable.Add(boneName, BasicBoneShader);
            ShaderTable.Add(noBoneName, BasicNoBoneShader);
        }

        public Shader(string name, string vsText, string psText, string vsEntrance, string psEntrance,bool isBoneModel)
        {
            m_name = name;
            m_vsText = vsText;
            m_psText = psText;
            m_vsEntrance = vsEntrance;
            m_psEntrance = psEntrance;

            if(isBoneModel)
            {
                if (D3DAPICall.CreateBonePipeline(m_name, m_vsText,
                        m_vsEntrance, m_psText, m_psEntrance) < 1)
                {
                    Console.WriteLine("Create bone pipeline fault: " + name);
                }
            }
            else
            {
                if (D3DAPICall.CreateNoBonePipeline(m_name, m_vsText,
                        m_vsEntrance, m_psText, m_psEntrance) < 1)
                {
                    Console.WriteLine("Create bone pipeline fault: " + name);
                }
            }

        }



        internal string m_name;
        internal string m_vsText;
        internal string m_psText;
        internal string m_vsEntrance;
        internal string m_psEntrance;

    }

}
