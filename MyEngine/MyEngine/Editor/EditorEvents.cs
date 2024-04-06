using CkfEngine.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Editor
{
    internal static class EditorEvents
    {
        internal static Action<string> OpenProject;

    }

    internal class EditorRunTime
    {
        private EditorRunTime() { }

        private static EditorRunTime instance;
        internal static EditorRunTime Instance
        {
            get { return instance == null ? instance = new EditorRunTime() : instance; }
        }

        internal Action UpdateEvent = null;

    }

}
