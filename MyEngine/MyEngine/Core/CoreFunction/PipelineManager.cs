using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal static class PipelineManager
    {
        internal static Dictionary<string, PipelineResource> PipelineTable = new Dictionary<string, PipelineResource>();

        internal static void CreatePipeline(bool isBoneModel, string name, string vsText, string psText, string vsEntrance, string psEntrance)
        {
            PipelineResource pipelineResource = new PipelineResource(isBoneModel, name, vsText, psText, vsEntrance, psEntrance);
            pipelineResource.Create();

            PipelineTable.Add(name, pipelineResource);
        }

        internal static void SetCameraTransform(Vector3 eye, Vector3 target, Vector3 up)
        {
            var pipelines = PipelineTable.Values.ToList();
            var pointers = pipelines.Select(item => item.GetResourcePointer()).ToArray();
            D3DAPICall.SetCameraTransform(eye,target,up, pointers, pointers.Length);
        }
    }

    internal class PipelineResource : RenderResource
    {
        private bool m_isBone;
        private string m_name;
        private string m_vsCode;
        private string m_vsEntry;
        private string m_psCode;
        private string m_psEntry;

        public PipelineResource(bool isBoneModel,string name,string vsText, string psText, string vsEntrance, string psEntrance)
        {
            m_isBone = isBoneModel;
            m_name = name;
            m_vsCode = vsText;
            m_vsEntry = vsEntrance;
            m_psCode = psText;
            m_psEntry = psEntrance;
        }


        protected override ulong CreateResource()
        {
            UInt64 pointer  =0;
            if(m_isBone)
            {
                if (D3DAPICall.CreateBonePipeline(m_name, m_vsCode,
                        m_vsEntry, m_psCode, m_psEntry,ref pointer) < 1)
                {
                    Console.WriteLine("Create bone pipeline fault: " + m_name);
                }
            }
            else
            {
                if (D3DAPICall.CreateNoBonePipeline(m_name, m_vsCode,
                        m_vsEntry, m_psCode, m_psEntry, ref pointer) < 1)
                {
                    Console.WriteLine("Create no bone pipeline fault: " + m_name);
                }
            }
            return pointer;
        }

        protected override void DeleteResource(ulong resPoint)
        {
            //wait
        }
    }
}
