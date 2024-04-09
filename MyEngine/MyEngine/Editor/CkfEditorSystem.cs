using System;
using System.Collections.Generic;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using CkfEngine.Core;

namespace CkfEngine.Editor
{
    public class CkfEditorSystem
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

        internal event Action UpdateEvent;

        private EditorCamera m_camera;

        internal void Init()
        {
            m_camera = new EditorCamera();
            m_camera.Init();
            m_camera.SetTransform(new Vector3(0, 20, -15), new Vector3(0, 0, 10), new Vector3(0, 1, 0));
            UpdateEvent += m_camera.Render;

            CoreEvents.CameraCreated += CameraCreated;

            ProjectManager.Instance.Init();
        }

        private void CameraCreated(Camera camera)
        {
            var panel = PanelRegister.GetExtendScreen();
            D3DAPICall.CreateRenderTarget(panel.Handle, camera.Uid, camera.m_width, camera.m_height);
            D3DAPICall.SetRenderTargetBackColor(camera.Uid,new float[4] {1.0f, 1.0f, 0.0f, 1.0f });
            camera.OwnerEntity.Transform.CalculateForwardAndUp();
            D3DAPICall.SetCameraTransform(
                    camera.OwnerEntity.Transform.Translation,
                    camera.OwnerEntity.Transform.m_forward,
                    camera.OwnerEntity.Transform.m_up);
            D3DAPICall.Render(camera.Uid);

            panel.Show();
        }

        internal void Update()
        {
            UpdateEvent?.Invoke();
        }

        public void SetEditorCamera(Vector3 eye, Vector3 target, Vector3 up)
        {
            m_camera.SetTransform(eye, target, up);
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
                D3DAPICall.CreateRenderTarget(PanelRegister.EditorMainScreen.Handle, m_uid, 800, 600);
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
                Implement();
                D3DAPICall.Render(m_uid);
            }
        }
    }
}
