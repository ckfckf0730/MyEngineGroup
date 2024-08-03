using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Runtime.Serialization.Formatters;
using System.Text;
using System.Text.RegularExpressions;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace CkfEngine.Core
{
    internal static class ShaderManager
    {
        public static void CreateShaderFile(string output)
        {
            ShaderBuilder shaderBuilder = new ShaderBuilder();
            RootParameter rootParameter = new RootParameter();
            rootParameter.name = "data1";
            rootParameter.dataType = "COLOR";
            rootParameter.descRangeType = D3D12_DESCRIPTOR_RANGE_TYPE.D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
            rootParameter.register = 3;
            rootParameter.visibility = D3D12_SHADER_VISIBILITY.D3D12_SHADER_VISIBILITY_PIXEL;
            rootParameter.defaultValue = (new Vector3(0,0,1)).ToString();
            shaderBuilder.Properties.Add(rootParameter);
            shaderBuilder.name = "TestShaderOut";
            shaderBuilder.vsCode = "vscode test .....";
            shaderBuilder.psCode = "vscode test .....";
            shaderBuilder.isBoneModel = true;

            var json = JsonConvert.SerializeObject(shaderBuilder, Formatting.Indented);

            File.WriteAllText(output, json);
        }

        public static void CreateShader(string file)
        {
            var text = File.ReadAllText(file);
            ShaderBuilder shaderBuilder = JsonConvert.DeserializeObject<ShaderBuilder>(text);

            //ShaderBuilder shaderBuilder = ShaderBuilder.Deserialize(text);


            Shader.CreateShaderByBuilder(shaderBuilder);
        }

        public static void CompileAllShader(string directory)
        {
            var files = Directory.GetFiles(directory);
            foreach(var file in files)
            {
                var extension = Path.GetExtension(file);
                if(extension.ToUpper() == ".SHADER")
                {
                    ShaderManager.CreateShader(file);
                }
            }


        }

    }


    public class Shader
    {
        internal static Shader BasicBoneShader;
        internal static Shader BasicNoBoneShader;
        internal static Dictionary<string,Shader> ShaderTable;

        public static Shader GetShader(string name)
        {
            Shader shader = null;
            ShaderTable.TryGetValue(name, out shader);
            return shader;
        }

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

        internal static void CreateShaderByBuilder(ShaderBuilder builder)
        {
            if(ShaderTable.ContainsKey(builder.name))
            {
                return;
            }

            var shader = new Shader(builder.name);
            if(builder.Properties != null)
            {
                foreach (var property in builder.Properties)
                {
                    shader.AddRootParameter(property.name, property.descRangeType,
                        property.register, property.visibility, property.dataType, property.defaultValue);
                }
            }
           
            shader.CreatePipeline(builder.vsCode, builder.psCode,
                builder.vsEntrance, builder.psEntrance, builder.isBoneModel);

            ShaderTable.Add(builder.name,shader);
        }

        internal string m_name;
        internal List<RootParameter> rootParameters;

        private Shader(string name)
        {
            m_name = name;
            rootParameters = new List<RootParameter>();
        }

        public void CreatePipeline(string vsText, string psText, string vsEntrance, string psEntrance, bool isBoneModel)
        {
            D3DAPICall.ClearRootSignatureSetting();
            foreach (var parameter in rootParameters)
            {
                D3DAPICall.SetRootSignature(parameter.name, 
                    parameter.descRangeType, parameter.register, parameter.visibility);
            }

            PipelineManager.Create(isBoneModel, m_name, vsText, psText, vsEntrance, psEntrance);
        }


        /// <summary>
        /// Should Add all Root Parameter before CreatePipeline.
        /// </summary>
        public void AddRootParameter(string name, D3D12_DESCRIPTOR_RANGE_TYPE rangeType, 
            int register, D3D12_SHADER_VISIBILITY visibility, string dataType, string defaultValue)
        {
            rootParameters.Add(new RootParameter()
            {
                name = name,
                descRangeType = rangeType,
                register = register,
                visibility = visibility,
                dataType = dataType,
                defaultValue = defaultValue
            });
        }
    }

    internal struct RootParameter
    {
        public string name;
        public D3D12_DESCRIPTOR_RANGE_TYPE descRangeType;
        public int register;
        public D3D12_SHADER_VISIBILITY visibility;
        public string dataType;
        public string defaultValue;
    }

    internal static class RootParameterType
    {
        internal static string Color = "float4";
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

    [Serializable]
    internal class ShaderBuilder
    {
        public string name;
        public List<RootParameter> Properties = new List<RootParameter>();
        public string vsCode;
        public string psCode;
        public string vsEntrance;
        public string psEntrance;
        public bool isBoneModel;

        public string Serialize()
        {
            string text = "";


            return text;
        }

        public static ShaderBuilder Deserialize(string text)
        {
            ShaderBuilder builder = new ShaderBuilder();

            var strings = CommonFuction.GetCurlyBracketsContents(text);
            string pattern = "\"([^\"]*)\"";
            Match match = Regex.Match(strings[0], pattern);
            builder.name = match.Groups[1].Value;

            strings = CommonFuction.GetCurlyBracketsContents(strings[1]);

            


            return builder;
        }
    }

}
