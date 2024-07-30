using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal abstract class RenderResource
    {
        private UInt64 m_resPoint;
        private State m_state = State.NotCreated;

        internal void Create()
        {
            m_resPoint = CreateResource();
            m_state = State.Created;
        }

        protected abstract UInt64 CreateResource();

        internal void Release()
        {
            DeleteResource(GetResourcePoint());

            m_state = State.Deleted;
            m_resPoint = 0;
        }

        protected abstract void DeleteResource(UInt64 resPoint);

        protected UInt64 GetResourcePoint()
        {
            if(m_state != State.Created)
            {
                throw new Exception("Resource State Error: "+ m_state);
            }
            return m_resPoint;
        }

        ~RenderResource()
        {
            if(m_state == State.Created)
            {
                Console.WriteLine("Worning! RenderResource release function not called and get GC: " + this.GetType());

                Release();
            }
        }

        enum State
        {
            NotCreated,
            Created,
            Deleted
        }
    }


    internal class RenderTargetResource : RenderResource
    {
        private System.IntPtr m_hwnd;
        private uint m_width;
        private uint m_height;

        internal RenderTargetResource(IntPtr hwnd, uint widt, uint height)
        {
            m_hwnd = hwnd;
            m_width = widt;
            m_height = height;
        }

        protected override ulong CreateResource()
        {
            ulong resPoint = 0;
            D3DAPICall.CreateRenderTarget(m_hwnd, m_width, m_height, ref resPoint);
            return resPoint;
        }

        protected override void DeleteResource(ulong resPoint)
        {
            D3DAPICall.DeleteRenderTarget(resPoint);
        }

        internal void SetRenderTargetBackColor(float[] color)
        {
            D3DAPICall.SetRenderTargetBackColor(GetResourcePoint(), color);
        }

        internal void Render()
        {
            D3DAPICall.Render(GetResourcePoint());
        }
    }

}
