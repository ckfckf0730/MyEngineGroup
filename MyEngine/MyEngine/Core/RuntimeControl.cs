using CkfEngine.Editor;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal class RuntimeControl
    {
        private RuntimeControl() { }

        private static RuntimeControl instance;
        internal static RuntimeControl Instance
        {
            get { return instance == null ? instance = new RuntimeControl() : instance; }
        }

        private bool m_isRun;
        private Scene m_scene;

        private IntPtr m_mainHandle = IntPtr.Zero;
        internal IntPtr MainHandle
        {
            get { return m_mainHandle; }
        }

        internal void SwitchState(IntPtr hwnd, Scene curScene)
        {
            if (!m_isRun)
            {
                Boot(hwnd, curScene);
            }
            else
            {
                Stop();
            }
        }

        internal void Boot(IntPtr hwnd, Scene curScene)
        {
            if (curScene == null)
            {
                Console.WriteLine("Cur Scene is null!");
                return;
            }
            m_scene = curScene;
            m_mainHandle = hwnd;
            m_isRun = true;
            curScene.Boot();
            CoreEvents.SceneRunned?.Invoke(curScene);
        }

        internal void Stop()
        {
            m_isRun = false;
            m_scene.Stop();
            CoreEvents.SceneRunned?.Invoke(m_scene);

            //Back to Scene initial state
            ProjectManager.Instance.ResetScene();
        }

        internal Action UpdateEvent = null;

        internal void Update(object sender, EventArgs e)
        {
            if(m_isRun)
            {
                UpdateEvent?.Invoke();
            }

        }

    }
}
