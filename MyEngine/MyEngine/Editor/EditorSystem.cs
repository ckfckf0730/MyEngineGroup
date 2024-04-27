using System;
using System.Collections.Generic;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using CkfEngine.Core;

namespace CkfEngine.Editor
{
    public class EditorSystem
    {
        private EditorSystem() { }

        public static EditorSystem Instance
        {
            get
            {
                return instance != null ? instance : (instance = new EditorSystem());
            }
        }
        private static EditorSystem instance;

        internal event Action UpdateEvent;

        private EditorCamera m_camera;

        internal void Init()
        {
            m_camera = new EditorCamera();
            m_camera.Init();
            m_camera.SetTransform(new Vector3(0, 10, -15), new Vector3(0, 0, 1), new Vector3(0, 1, 0));
            UpdateEvent += m_camera.Render;

            CoreEvents.CameraCreated += CameraCreated;
            CoreEvents.CameraDestoried += CameraDestoried;

            ProjectManager.Instance.Init();

            PanelRegister.EditorMainScreen.MouseDown += SceneWindowMouseDown;
            PanelRegister.EditorMainScreen.MouseUp += SceneWindowMouseUp;
            PanelRegister.EditorMainScreen.MouseMove += SceneWindowMouseMove;

            PanelRegister.EditorMainScreen.KeyDown += KeyDown;
        }

        private bool m_isRightDown = false;
        private System.Drawing.Point m_lastPoint;

        private void KeyDown(object sender, KeyEventArgs e)
        {
            float moveScalse = 0.05f;

            if (e.KeyCode == Keys.W)
            {
                m_camera.Move(new Vector3(0, 0, moveScalse));
            }
            else if (e.KeyCode == Keys.S)
            {
                m_camera.Move(new Vector3(0, 0, -1 * moveScalse));
            }
            else if (e.KeyCode == Keys.A)
            {
                m_camera.Move(new Vector3( -1 * moveScalse, 0, 0));
            }
            else if (e.KeyCode == Keys.D)
            {
                m_camera.Move(new Vector3(moveScalse, 0, 0));
            }
        }

        private void SceneWindowMouseDown(object sender, MouseEventArgs e)
        {
            PanelRegister.EditorMainScreen.Focus();
            if (e.Button == MouseButtons.Right)
            {
                m_isRightDown = true;
                m_lastPoint = e.Location;
            }
        }
        private void SceneWindowMouseUp(object sender, MouseEventArgs e)
        {
            if (e.Button == MouseButtons.Right)
            {
                m_isRightDown = false;
            }
        }
        private void SceneWindowMouseMove(object sender, MouseEventArgs e)
        {
            if(m_isRightDown)
            {
                float rotScale = 0.005f;
                float xOffset = (e.Location.X - m_lastPoint.X) * rotScale;
                float yOffset = (e.Location.Y - m_lastPoint.Y) * rotScale;

                m_lastPoint = e.Location;

                Vector3 rotation = new Vector3(yOffset,xOffset,0);
                m_camera.SetRot(rotation);
            }
        }

        private Dictionary<ulong,Control> m_cameraPanelTable = new Dictionary<ulong,Control>();

        private void CameraCreated(Camera camera)
        {
            if(ProjectManager.Instance.CurScene != null &&
                ProjectManager.Instance.CurScene.MainCamera ==null)
            {
                ProjectManager.Instance.CurScene.MainCamera = camera;
            }

            var panel = PanelRegister.GetExtendScreen();
            m_cameraPanelTable.Add(camera.Uid, panel);
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

        private void CameraDestoried(Camera camera)
        {
            D3DAPICall.DeleteRenderTarget(camera.Uid);

            Control panel;
            m_cameraPanelTable.TryGetValue(camera.Uid, out panel);
            if(panel != null )
            {
                PanelRegister.DeleteExtendScreen(panel);
                m_cameraPanelTable.Remove(camera.Uid);
            }

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
            public Vector3 Forward;
            public Vector3 Up;

            public float FovAngleY;  //View angle
            public float AspectRatio;      //width : height
            public float NearZ;
            public float Far;

            private ulong m_uid =99999989; //temp editor camera uid

            public void Init()
            {
                D3DAPICall.CreateRenderTarget(PanelRegister.EditorMainScreen.Handle, m_uid, 800, 600);
            }

            public void SetTransform(Vector3 eye, Vector3 forward, Vector3 up)
            {
                Eye = eye;
                Forward = forward;
                Up = up;
                Implement();
            }

            public void Move(Vector3 dir)
            {
                var right = GraphAlgorithms.RotateFrom3Axis(Forward, new Vector3(0, (float)(Math.PI / 2), 0));
                right.Y = 0;
                right = Vector3.Normalize(right);
                Forward = Vector3.Normalize(Forward);
                //Matrix4x4.CreateWorld dose use the right hand coord system, but this engine use left hand one. so make a reversed forward.+
                var forward = -Forward;
                Vector3 selfUp = Vector3.Cross(Forward, right);

                Matrix4x4 rotationMatrix = Matrix4x4.CreateWorld(Vector3.Zero, forward, selfUp);

                Vector3 moveVec = Vector3.Transform(dir, rotationMatrix);

                Eye += moveVec;
                Implement();
            }

            public void SetRot(Vector3 rot)
            {
                Matrix4x4 mat = GraphAlgorithms.MatRotateFrom3Axis(rot);

                Forward = Vector3.Transform(Forward, mat);
                //Up = Vector3.Transform(Up, mat);

                Implement();
            }

            private void Implement()
            {
                Vector3 target = Forward + Eye;
                D3DAPICall.SetCameraTransform(Eye, target, Up);
            }

            public void Render()
            {
                Implement();
                D3DAPICall.Render(m_uid);
            }
        }
    }
}
