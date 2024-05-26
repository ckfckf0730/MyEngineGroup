using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Runtime.Serialization.Formatters;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

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
            BasicBoneShader = new Shader(boneName);
            BasicBoneShader.CreatePipeline(vsCode, psCode, "BasicVS", "BasicPS", true);

            vsCode = File.ReadAllText("NoBoneVertexShader.hlsl");
            BasicNoBoneShader = new Shader(noBoneName);
            BasicNoBoneShader.CreatePipeline(vsCode, psCode, "BasicVS", "BasicPS", false);

            ShaderTable.Add(boneName, BasicBoneShader);
            ShaderTable.Add(noBoneName, BasicNoBoneShader);
        }

        internal string m_name;
        internal List<RootParameter> rootParameters;
        //internal string m_vsText;
        //internal string m_psText;
        //internal string m_vsEntrance;
        //internal string m_psEntrance;

        public Shader(string name)
        {
            m_name = name;
            rootParameters = new List<RootParameter>();
            //m_vsText = vsText;
            //m_psText = psText;
            //m_vsEntrance = vsEntrance;
            //m_psEntrance = psEntrance;
        }

        public bool CreatePipeline(string vsText, string psText, string vsEntrance, string psEntrance, bool isBoneModel)
        {
            D3DAPICall.ClearRootSignatureSetting();
            foreach (var parameter in rootParameters)
            {
                D3DAPICall.SetRootSignature(parameter.name, 
                    parameter.descRangeType, parameter.register, parameter.visibility);
            }

            if (isBoneModel)
            {
                if (D3DAPICall.CreateBonePipeline(m_name, vsText,
                        vsEntrance, psText, psEntrance) < 1)
                {
                    Console.WriteLine("Create bone pipeline fault: " + m_name);
                    return false;
                }
            }
            else
            {
                if (D3DAPICall.CreateNoBonePipeline(m_name, vsText,
                        vsEntrance, psText, psEntrance) < 1)
                {
                    Console.WriteLine("Create bone pipeline fault: " + m_name);
                    return false;
                }
            }
            return true;
        }


        /// <summary>
        /// Should Add all Root Parameter before CreatePipeline.
        /// </summary>
        public void AddRootParameter(string name, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, 
            int register, D3D12_SHADER_VISIBILITY visibility, Type dataType)
        {
            rootParameters.Add(new RootParameter()
            {
                name = name,
                descRangeType = rangeType,
                register = register,
                visibility = visibility,
                dataType = dataType
            });
        }
    }

    internal struct RootParameter
    {
        internal string name;
        internal D3D12_DESCRIPTOR_RANGE_TYPE descRangeType;
        internal int register;
        internal D3D12_SHADER_VISIBILITY visibility;

        internal Type dataType;
    }

    public enum D3D12_DESCRIPTOR_RANGE_TYPE
    {
        D3D12_DESCRIPTOR_RANGE_TYPE_SRV = 0,
        D3D12_DESCRIPTOR_RANGE_TYPE_UAV = (D3D12_DESCRIPTOR_RANGE_TYPE_SRV + 1),
        D3D12_DESCRIPTOR_RANGE_TYPE_CBV = (D3D12_DESCRIPTOR_RANGE_TYPE_UAV + 1),
        D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER = (D3D12_DESCRIPTOR_RANGE_TYPE_CBV + 1)
    }

    public enum D3D12_SHADER_VISIBILITY
    {
        D3D12_SHADER_VISIBILITY_ALL = 0,
        D3D12_SHADER_VISIBILITY_VERTEX = 1,
        D3D12_SHADER_VISIBILITY_HULL = 2,
        D3D12_SHADER_VISIBILITY_DOMAIN = 3,
        D3D12_SHADER_VISIBILITY_GEOMETRY = 4,
        D3D12_SHADER_VISIBILITY_PIXEL = 5,
        D3D12_SHADER_VISIBILITY_AMPLIFICATION = 6,
        D3D12_SHADER_VISIBILITY_MESH = 7
    }

}
