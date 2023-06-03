using System;
using System.Collections.Generic;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Editor
{
    internal class CkfEditorSystem
    {
        private CkfEditorSystem() { }

        public static CkfEditorSystem Instance
        {
            get
            {
                return instance != null ? instance : (instance = new CkfEditorSystem());
            }
        }
        private static CkfEditorSystem instance;

        public event Action UpdateEvent;

        private EditorCamera m_camera;
        public void Init()
        {
            m_camera = new EditorCamera();
            m_camera.Init();
            m_camera.SetTransform(new Vector3(0, 20, -15), new Vector3(0, 0, 10), new Vector3(0, 1, 0));
            UpdateEvent += m_camera.Render;
        }

        internal void Update()
        {
            UpdateEvent?.Invoke();
        }



        private class EditorCamera
        {
            public Vector3 Eye;
            public Vector3 Target;
            public Vector3 Up;

            public float FovAngleY;  //View angle
            public float AspectRatio;      //width : height
            public float NearZ;
            public float Far;

            private ulong m_uid;

            public void Init()
            {
                m_uid = UID.GetUID();
                D3DAPICall.CreateRenderTarget(PanelRegister.EditorMainScreen.Handle, m_uid, 800, 600);
                D3DAPICall.SetCameraProjection((float)(Math.PI / 2), 800.0f / 600.0f, 1.0f, 100.0f);
            }

            public void SetTransform(Vector3 eye, Vector3 target, Vector3 up)
            {
                Eye = eye; 
                Target = target;
                Up = up;
                Implement();
            }

            private void Implement()
            {
                D3DAPICall.SetCameraTransform(Eye, Target, Up);
            }

            public void Render()
            {
                D3DAPICall.Render(m_uid);
            }
        }
    }
}
