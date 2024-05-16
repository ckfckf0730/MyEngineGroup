using System;
using System.Collections.Generic;
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
        internal string m_name;
        internal string text;
    }

}
