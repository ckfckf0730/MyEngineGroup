using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal class EngineRunTime
    {
        private EngineRunTime() { }

        private static EngineRunTime instance;
        internal static EngineRunTime Instance
        {
            get { return instance == null ? instance = new EngineRunTime() : instance; }
        }


        private List<Component> m_components = new List<Component>();



        internal void Boot(IntPtr hwnd,uint witdh,uint height)
        {
            //test camera UID
            ulong uid = 9999999;
            D3DAPICall.CreateRenderTarget(hwnd, uid, witdh, height);
            //D3DAPICall.SetCameraProjection((float)(Math.PI / 2), 800.0f / 600.0f, 1.0f, 100.0f);  //test parameter
        }

        internal Action UpdateEvent = null;

        internal void Register(Component component)
        {
            m_components.Add(component);
        }

        internal void Update()
        {
            UpdateEvent?.Invoke();
        }
    }
}
