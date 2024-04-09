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

        private IntPtr m_mainHandle = IntPtr.Zero;
        internal IntPtr MainHandle
        {
            get { return m_mainHandle; }
        }


        internal void Init(IntPtr hwnd)
        {
            m_mainHandle = hwnd;
        }

        internal Action UpdateEvent = null;

        internal void Register(Component component)
        {
            m_components.Add(component);
        }

        internal void Update(object sender, EventArgs e)
        {
            UpdateEvent?.Invoke();
        }

    }
}
