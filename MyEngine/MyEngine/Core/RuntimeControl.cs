using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Linq;
using System.Numerics;
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

            TestBoot();
        }

        internal void Stop()
        {
            m_isRun = false;
            m_scene.Stop();
            CoreEvents.SceneStoped?.Invoke(m_scene);
        }

        internal Action UpdateEvent = null;

        internal void Update(object sender, EventArgs e)
        {
            if(m_isRun)
            {
                UpdateEvent?.Invoke();
                TestUpdate();
            }

        }

        ModelBoneRenderer renderer;
        private void TestBoot()
        {
            //var entities = Entity.GetAllEntities();

            //foreach(var entity in entities)
            //{
            //    if(entity.Name == "Role")
            //    {
            //        renderer = entity.GetComponent<ModelBoneRenderer>();
            //        renderer.ChangeShader(3, Shader.ShaderTable["TestShader"]);
                    

            //        return;
            //    }
            //}
        }

        float yAxis = 0;

        private void TestUpdate()
        {
            //if(renderer != null)
            //{
            //    yAxis += 0.1f;
            //    Vector3 lightDir = new Vector3(1, -1, 1);
            //    Matrix4x4 rotY = Matrix4x4.CreateRotationY(yAxis);
            //    lightDir = Vector3.Transform(lightDir, rotY);

            //    MaterialManager.SetCustomizedResourceValue(renderer.Materials[3], "testFloat3", lightDir);
            //}
        }
    }
}
