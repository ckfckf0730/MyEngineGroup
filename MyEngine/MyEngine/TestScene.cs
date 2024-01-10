using CkfEngine.Core;
using CkfEngine.Editor;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine
{
    internal class TestScene
    {
        private  Entity m_camera;
        private Entity m_role1;
        private Entity m_testModel;

        private ulong m_model1;
        private float angle;

        private ulong m_model2;

        public void Init()
        {
            m_role1 = Entity.CreateEntity("role1");
            //m_role1.Transform.Rotation = new Vector3(0, (float)(Math.PI * 1) / 2, 0);
            var renderer = m_role1.CreateComponent<ModelBoneRenderer>();
            renderer.SetPMDModel("Model/初音ミク.pmd");
            D3DAPICall.LoadAnimation(m_role1.Uid, "motion/motion.vmd");

            m_testModel = Entity.CreateEntity("testModel");
            var renderer2 = m_testModel.CreateComponent<ModelRenderer>();
            renderer2.SetModel("Model/Cube.vd");

            m_testModel = Entity.CreateEntity("testModel2");
            var renderer3 = m_testModel.CreateComponent<ModelRenderer>();
            renderer3.SetModel("Model/Cube.vd");
            m_testModel.Transform.Translation = new Vector3(0, 3, 0);


            CkfEditorSystem.Instance.SetEditorCamera(
                new Vector3(0, 12, -20), new Vector3(0, 12, 0), new Vector3(0, 1, 0));


            var role2 = Prefab.ReadPrefabFile("role1.prefab");
            role2.Transform.Translation = new Vector3(10, 0, 0);

            renderer = role2.GetComponent<ModelBoneRenderer>();
            renderer?.SetPMDModel("Model/初音ミク.pmd");

            //m_model1 = UID.GetUID();
            //if (D3DAPICall.SetModel(m_model1, "Model/初音ミク.pmd") < 1)
            //{
            //    Console.WriteLine("InitD3d fault");
            //}
            //D3DAPICall.LoadAnimation(m_model1, "motion/pose.vmd");


        }

        public void TestUpdate()
        {
            //var worldMat = Matrix4x4.CreateRotationY(angle);
            //angle += 0.1f;

            //var offMat = Matrix4x4.CreateTranslation(-5,0,0);

            //worldMat= worldMat * offMat;

            D3DAPICall.UpdateAnimation(m_role1.Uid);
        }


    }
}
