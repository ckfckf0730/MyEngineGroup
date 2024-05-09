using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal class MaterialManager
    {
        

    }

    internal class StandardMaterial : MaterialBase
    {
        public Vector3 diffuse;
        public float alpha;
        public float specularity;
        public Vector3 specular;
        public Vector3 ambient;
        public byte toonIdx;
        public byte edgeFlg;
        public uint indicesNum;
        public string texFilePath;  //20bytes
    }

    internal class MaterialBase
    {
        public Shader shader;
    }

    

}
