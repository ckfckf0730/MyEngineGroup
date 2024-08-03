using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace CkfEngine.Core
{
    internal static class PipelineManager
    {
        internal static Dictionary<string, ResourceHandle> PipelineTable = new Dictionary<string, ResourceHandle>();

        internal static ResourceHandle Create(bool isBoneModel, string name, string vsText, string psText, string vsEntrance, string psEntrance)
        {
            ulong pointer = 0;
            if (isBoneModel)
            {
                if (D3DAPICall.CreateBonePipeline(name, vsText,
                        vsEntrance, psText, psEntrance, ref pointer) < 1)
                {
                    Console.WriteLine("Create bone pipeline fault: " + name);
                    return new ResourceHandle();
                }
            }
            else
            {
                if (D3DAPICall.CreateNoBonePipeline(name, vsText,
                        vsEntrance, psText, psEntrance, ref pointer) < 1)
                {
                    Console.WriteLine("Create no bone pipeline fault: " + name);
                    return new ResourceHandle();
                }
            }

            var handle = ResourceHandle.CreateHandle(pointer, ResourceHandle.ResourceType.Pipeline);
            PipelineTable.Add(name, handle);
            return handle;           
        }

        internal static void Release(ref ResourceHandle handle)
        {
            //wait for....
        }

        internal static void SetCameraTransform(Vector3 eye, Vector3 target, Vector3 up)
        {
            var pipelines = PipelineTable.Values.ToList();
            var pointers = pipelines.Select(item => item.GetPointer()).ToArray();
            D3DAPICall.SetCameraTransform(eye,target,up, pointers, pointers.Length);
        }
    }


}
