using CkfEngine.Core;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Numerics;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;
using static System.Windows.Forms.VisualStyles.VisualStyleElement.TextBox;

namespace CkfEngine.Core
{
    [Serializable]
    public abstract class Component : EngineObject
    {
        private Entity m_owner;
        [JsonIgnore]
        public Entity OwnerEntity
        { get { return m_owner; } }

        protected virtual void OnCreated() { }

        protected virtual void OnDestroyed() { }

        protected virtual void Update() { }

        internal void BindEntity(Entity entity)
        {
            RuntimeControl.Instance.UpdateEvent += Update;

            m_owner = entity;
            try
            {
                OnCreated();
            }
            catch (Exception ex)
            {
                Console.WriteLine(ex.Message);
                Console.WriteLine(ex.StackTrace);
            }
        }

        internal void Release()
        {
            RuntimeControl.Instance.UpdateEvent -= Update;
            OnDestroyed();
            m_owner = null;
        }
    }

    public class Transform : Component
    {
        //private Matrix4x4 m_matrix;
        //public Matrix4x4 Matrix { get { return m_matrix; } }
        [MyAttributeShowInspector]
        private Vector3 m_translation;
        public Vector3 Translation
        {
            get { return m_translation; }
            set { m_translation = value; EffectiveTransform(); }
        }
        [MyAttributeShowInspector]
        private Vector3 m_rotation;
        public Vector3 Rotation
        {
            get { return m_rotation; }
            set { m_rotation = value; EffectiveTransform(); }
        }
        [MyAttributeShowInspector]
        private Vector3 m_scale = Vector3.One;
        public Vector3 Scale
        {
            get { return m_scale; }
            set { m_scale = value; EffectiveTransform(); }
        }


        internal Vector3 m_forward;
        internal Vector3 m_up;

        [JsonProperty]
        internal ulong m_parentUid;

        private Transform m_parent;
        public Transform Parent { get { return m_parent; } }
        private List<Transform> m_children = new List<Transform>();

        [JsonIgnore]
        public Transform[] Children
        {
            get { return m_children.ToArray(); }
        }

        internal void EffectiveTransform()
        {
            var worldMat = Matrix4x4.CreateScale(m_scale) *
                Matrix4x4.CreateFromYawPitchRoll(m_rotation.Y, m_rotation.X, m_rotation.Z) *
                Matrix4x4.CreateTranslation(m_translation);

            if (OwnerEntity != null)
            {
                D3DAPICall.SetModelTransform(OwnerEntity.Uid, worldMat);
            }

            EventValueChanged?.Invoke(this);
        }

        public void SetParent(Transform parent)
        {
            var temp = parent;
            int timeOut = 0;
            while (temp != null)
            {
                if (temp == this)
                {
                    Console.WriteLine("SetParent Error!! roop reference!!   " + this.Name);
                }
                temp = temp.Parent;
                timeOut++;

                if (timeOut > 10000)
                {
                    throw new Exception("SetParent Time Out " + this.Name);
                }
            }

            UnsetParent();
            if (parent != null && parent != this)
            {
                m_parent = parent;
                m_parentUid = parent.Uid;
                parent.m_children.Add(this);

            }
            EventSetParent?.Invoke(OwnerEntity.Uid, parent == null ? 0 : parent.OwnerEntity.Uid,
                false, OwnerEntity.Name);
        }

        private void UnsetParent()
        {
            if (m_parent != null)
            {
                m_parent.m_children.Remove(this);
                m_parent = null;

            }
        }

        internal void CalculateForwardAndUp()
        {
            var mat = GraphAlgorithms.MatRotateFrom3Axis(this.OwnerEntity.Transform.Rotation);

            Vector3 forward = new Vector3(0, 0, 1);
            Vector3 up = new Vector3(0, 1, 0);

            m_forward = Vector3.Transform(forward, mat);
            m_up = Vector3.Transform(up, mat);
        }

        protected override void OnDestroyed()
        {
            base.OnDestroyed();

            UnsetParent();
            EventSetParent?.Invoke(OwnerEntity.Uid, 0, true, OwnerEntity.Name);
        }

        // entity UID, parent UID(null = 0), is delete
        internal static event Action<ulong, ulong, bool, string> EventSetParent;
        internal event Action<Transform> EventValueChanged;
    }

    public class Camera : Component
    {
        internal static Dictionary<ulong, Camera> CameraTable = new Dictionary<ulong, Camera>();

        [JsonProperty]
        internal float m_fovAngleY;
        [JsonProperty]
        internal uint m_width;
        [JsonProperty]
        internal uint m_height;
        [JsonProperty]
        internal float m_near;
        [JsonProperty]
        internal float m_far;

        public Camera()
        {
            m_fovAngleY = (float)Math.PI / 2;
            m_width = 800;
            m_height = 600;
            m_near = 1.0f;
            m_far = 100.0f;
        }

        protected override void OnCreated()
        {
            CoreEvents.CameraCreated?.Invoke(this);
            CameraTable.Add(Uid, this);

            this.OwnerEntity.Transform.EventValueChanged += TransOnChanged;

            //D3DAPICall.CreateRenderTarget(PanelRegister.EditorMainScreen.Handle, this.Uid, 800, 600);
            //D3DAPICall.SetCameraProjection((float)(Math.PI / 2), 800.0f / 600.0f, 1.0f, 100.0f);
        }

        private void TransOnChanged(Transform trans)
        {
            trans.CalculateForwardAndUp();
            Vector3 target = trans.m_forward + trans.Translation;
            Vector3 up = trans.m_up;

            D3DAPICall.SetCameraTransform(
                    trans.Translation,
                    target,
                    up);
            D3DAPICall.Render(this.Uid);
        }

        protected override void OnDestroyed()
        {
            CoreEvents.CameraDestoried?.Invoke(this);
            this.OwnerEntity.Transform.EventValueChanged -= TransOnChanged;

            CameraTable.Remove(Uid);
        }

        private void Implement()
        {
            var trans = OwnerEntity.Transform;
            Vector3 target = trans.m_forward + trans.Translation;
            D3DAPICall.SetCameraTransform(trans.Translation, target, trans.m_up);
        }

        protected override void Update()
        {
            Implement();
            D3DAPICall.Render(this.Uid);
        }

    }

    [Serializable]
    public class FileLoad
    {
        [JsonProperty]
        private string fullpath;
        [JsonIgnore]
        public string FullPath
        {
            set { fullpath = value; OnChenged?.Invoke(); }
            get { return fullpath; }
        }

        public event Action OnChenged;
    }

    [Serializable]
    public class ModelBoneRenderer : Component
    {
        [JsonProperty]
        [MyAttributeLoadFileType("PMD")]
        [MyAttributeShowInspector]
        private FileLoad m_file;
        [JsonIgnore]
        public FileLoad File
        { get { return m_file; } }

        private PMDModel m_model;
        private bool m_isLoaded;
        internal PMDModelInstance m_pmdModelInstance;

        protected override void OnCreated()
        {
            m_isLoaded = false;
            if (m_file == null)
            {
                m_file = new FileLoad();

            }
            else
            {
                var path = m_file.FullPath;
                m_file = new FileLoad();
                m_file.FullPath = path;
                SetPMDModel();
            }
            m_file.OnChenged += SetPMDModel;
        }

        protected override void OnDestroyed()
        {
            D3DAPICall.DeleteModelInstance(OwnerEntity.Uid);
        }

        internal static uint testMatId = 0;

        private void SetPMDModel()
        {
            if (m_isLoaded)
            {
                m_isLoaded = false;
            }

            if (ModelManager.LoadPMDFile(m_file.FullPath, out m_model))
            {
                if (ModelManager.SetPMDVertices(m_file.FullPath, m_model))
                {
                    if (ModelManager.SetMaterials(testMatId++, "TestShader", m_model))
                    {
                        if (ModelManager.SetPMDBoneIk(m_file.FullPath, m_model))
                        {
                            if (ModelManager.InstantiatePMDModel(OwnerEntity.Uid, m_file.FullPath, m_model.m_boneCount))
                            {
                                m_pmdModelInstance = new PMDModelInstance(m_model, OwnerEntity.Uid);
                                D3DAPICall.CreateCustomizedResource(OwnerEntity.Uid, "testColor", 12, 3);
                                Vector3 color = new Vector3(0.0f, 0, 0);
                                var data = CommonFuction.StructToByteArray(color);
                                D3DAPICall.SetCustomizedResourceValue(OwnerEntity.Uid, "testColor", data);

                                D3DAPICall.BindPipeline(OwnerEntity.Uid, "TestShader");
                                //D3DAPICall.BindPipeline(OwnerEntity.Uid, Shader.BasicBoneShader.m_name);
                                D3DAPICall.BindMaterialControl(OwnerEntity.Uid, testMatId - 1);
                                m_isLoaded = true;



                                
                            }
                        }
                    }
                }
            }

            //var result = D3DAPICall.SetPMDModel(OwnerEntity.Uid, m_file.FullPath);
            //if (result == 1)
            //{
            //    m_isLoaded = true;
            //}

            OwnerEntity.Transform.EffectiveTransform();
        }

        public void SetPMDModel(string path)
        {
            m_file.FullPath = path;
        }


    }

    [Serializable]
    public class ModelRenderer : Component
    {
        [MyAttributeLoadFileType("VD")]
        [MyAttributeShowInspector]
        [JsonProperty]
        private FileLoad m_file;
        [JsonIgnore]
        public FileLoad File
        { get { return m_file; } }

        private bool m_isLoaded;

        private Model m_model;
        internal ModelInstance m_modelInstance;

        protected override void OnCreated()
        {
            m_isLoaded = false;
            if (m_file == null)
            {
                m_file = new FileLoad();

            }
            else
            {
                var path = m_file.FullPath;
                m_file = new FileLoad();
                m_file.FullPath = path;
                SetModel();
            }
            m_file.OnChenged += SetModel;
        }

        protected override void OnDestroyed()
        {
            base.OnDestroyed();

            D3DAPICall.DeleteModelInstance(OwnerEntity.Uid);
        }

        private void SetModel()
        {
            if (m_isLoaded)
            {
                //remove currently model


                m_isLoaded = false;
            }

            if (ModelManager.LoadVDFile(m_file.FullPath, out m_model))
            {
                if (ModelManager.SetVDVertices(m_file.FullPath, m_model))
                {

                    if (ModelManager.InstantiateVDModel(OwnerEntity.Uid, m_file.FullPath))
                    {
                        m_modelInstance = new ModelInstance(m_model, OwnerEntity.Uid);
                        D3DAPICall.BindPipeline(OwnerEntity.Uid, Shader.BasicNoBoneShader.m_name);
                        m_isLoaded = true;
                    }

                }
            }


            OwnerEntity.Transform.EffectiveTransform();
        }

        public void SetModel(string path)
        {
            m_file.FullPath = path;
        }


    }

    public class Animation : Component
    {
        [JsonProperty]
        [MyAttributeLoadFileType("VMD")]
        [MyAttributeShowInspector]
        private FileLoad m_file;
        [JsonIgnore]
        public FileLoad File
        { get { return m_file; } }

        private bool m_isLoaded;
        private AnimationControl m_animaionControl;

        protected override void OnCreated()
        {
            m_isLoaded = false;
            if (m_file == null)
            {
                m_file = new FileLoad();

            }
            else
            {
                var path = m_file.FullPath;
                m_file = new FileLoad();
                m_file.FullPath = path;
                SetAnimation();
            }
            m_file.OnChenged += SetAnimation;
        }

        protected override void OnDestroyed()
        {
            var boneRenderer = OwnerEntity.GetComponent<ModelBoneRenderer>();
            if (boneRenderer != null)
            {
                boneRenderer.File.OnChenged -= BoneRendererChanged;
            }
        }

        private void SetAnimation()
        {
            if (m_isLoaded)
            {
                //remove currently 


                m_isLoaded = false;
            }

            VMDAnimation vmdAnime;
            var boneRenderer = OwnerEntity.GetComponent<ModelBoneRenderer>();
            boneRenderer.File.OnChenged += BoneRendererChanged;

            if (ModelManager.LoadVMDFile(m_file.FullPath, out vmdAnime) &&
                boneRenderer != null)
            {
                m_animaionControl = new AnimationControl(vmdAnime, boneRenderer.m_pmdModelInstance);
                m_animaionControl.StartAnimation();

                m_isLoaded = true;
            }
        }

        private void BoneRendererChanged()
        {
            var boneRenderer = OwnerEntity.CreateComponent<ModelBoneRenderer>();
            m_animaionControl.SetModelInstance(boneRenderer.m_pmdModelInstance);
        }

        public void SetAnimation(string path)
        {
            m_file.FullPath = path;
        }

        protected override void Update()
        {
            m_animaionControl.UpdateAnimation();
        }
    }




    [AttributeUsage(AttributeTargets.Field, Inherited = true, AllowMultiple = true)]
    sealed class MyAttributeLoadFileType : Attribute
    {
        public string Description { get; }

        public MyAttributeLoadFileType(string description)
        {
            Description = description;
        }
    }

    [AttributeUsage(AttributeTargets.Field, Inherited = true, AllowMultiple = true)]
    sealed class MyAttributeShowInspector : Attribute
    {
        public MyAttributeShowInspector()
        {

        }
    }

    [AttributeUsage(AttributeTargets.Class, Inherited = true, AllowMultiple = true)]
    sealed class MyAttributeHideAddComponent : Attribute
    {
        public MyAttributeHideAddComponent()
        {

        }
    }
}
