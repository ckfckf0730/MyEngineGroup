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
