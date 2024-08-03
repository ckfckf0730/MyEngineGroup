using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal static class RenderTargetManager
    {
        internal static ResourceHandle Create(IntPtr hwnd, uint width, uint height)
        {
            ulong resPointer = 0;
            int resual = D3DAPICall.CreateRenderTarget(hwnd, width, height, ref resPointer);
            if (resual != 1)
            {
                return new ResourceHandle();
            }

            return ResourceHandle.CreateHandle(resPointer, ResourceHandle.ResourceType.RenderTarget);
        }

        internal static void Release(ref ResourceHandle resHandle)
        {
            D3DAPICall.DeleteRenderTarget(resHandle.GetPointer());
            resHandle.Release();
        }

        internal static void SetRenderTargetBackColor(float[] color, ResourceHandle resHandle)
        {
            D3DAPICall.SetRenderTargetBackColor(resHandle.GetPointer(), color);
        }

        internal static void Render(ResourceHandle resHandle)
        {
            var cameraPointer = resHandle.GetPointer();
            D3DAPICall.RenderTargetClear(cameraPointer);

            foreach (var pipeline in PipelineManager.PipelineTable.Values)
            {
                D3DAPICall.DrawPipeline(pipeline.GetPointer());
            }

            D3DAPICall.RenderTargetFlip(cameraPointer);
        }

        private static bool CheckResource(ref ResourceHandle resHandle)
        {
            if(resHandle.GetPointer() == 0 || 
                resHandle.GetResourceType() != ResourceHandle.ResourceType.RenderTarget)
            {
                return false;
            }

            return true;
        }
    }
}
