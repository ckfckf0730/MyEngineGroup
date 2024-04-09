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
            EngineRunTime.Instance.UpdateEvent += Update;

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
            EngineRunTime.Instance.UpdateEvent -= Update;
            OnDestroyed();
            m_owner = null;
        }
    }

    public class Transform : Component
    {
        //private Matrix4x4 m_matrix;
        //public Matrix4x4 Matrix { get { return m_matrix; } }
        private Vector3 m_translation;
        public Vector3 Translation
        {
            get { return m_translation; }
            set { m_translation = value; EffectiveTransform(); }
        }
        private Vector3 m_rotation;
        public Vector3 Rotation
        {
            get { return m_rotation; }
            set { m_rotation = value; EffectiveTransform(); }
        }
        private Vector3 m_scale = Vector3.One;

        internal Vector3 m_forward;
        internal Vector3 m_up;

        public Vector3 Scale
        {
            get { return m_scale; }
            set { m_scale = value; EffectiveTransform(); }
        }

        [JsonProperty]
        internal ulong m_parentUid;

        private Transform m_parent;
        public Transform Parent { get { return m_parent; } }
        private List<Transform> m_children = new List<Transform>();

        [JsonIgnore]
        public Transform[] Children
        {
            get { return m_children.ToArray();}
        }

        internal void EffectiveTransform()
        {
            var worldMat = Matrix4x4.CreateScale(m_scale) *
                Matrix4x4.CreateFromYawPitchRoll(m_rotation.Y, m_rotation.X, m_rotation.Z) *
                Matrix4x4.CreateTranslation(m_translation);

            if(OwnerEntity!= null)
            {
                D3DAPICall.SetModelTransform(OwnerEntity.Uid, worldMat);
            }

        }

        public void SetParent(Transform parent)
        {
            var temp = parent;
            int timeOut = 0;
            while (temp != null)
            {
                if(temp == this)
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
            if(m_parent != null)
            {
                m_parent.m_children.Remove(this);
                m_parent = null;

            }
        }

        internal void CalculateForwardAndUp()
        {
            Vector3 rotation = this.OwnerEntity.Transform.Rotation;
            Matrix4x4 rY = Matrix4x4.CreateFromAxisAngle(new Vector3(0, 1, 0), rotation.Y);
            Matrix4x4 rX = Matrix4x4.CreateFromAxisAngle(new Vector3(1, 0, 0), rotation.X);
            Matrix4x4 rZ = Matrix4x4.CreateFromAxisAngle(new Vector3(0, 0, 1), rotation.Z);

            Matrix4x4 m = rZ * rX * rY;

            Vector3 forward = new Vector3(0, 0, 1);
            Vector3 up = new Vector3(0, 1, 0);

            m_forward = Vector3.Transform(forward, m);
            m_up = Vector3.Transform(up, m);
        }

        protected override void OnDestroyed()
        {
            base.OnDestroyed();

            UnsetParent();
            EventSetParent?.Invoke(OwnerEntity.Uid,0,true, OwnerEntity.Name);
        }

        // entity UID, parent UID(null = 0), is delete
        internal static event Action<ulong, ulong, bool, string> EventSetParent;
    }

    public class Camera : Component
    {
        internal float m_fovAngleY;
        internal uint m_width;
        internal uint m_height;
        internal float m_near;
        internal float m_far; 

        public Camera()
        {
            m_fovAngleY = (float)Math.PI / 2;
            m_width = 800;
            m_height= 600;
            m_near = 1.0f;
            m_far = 100.0f;
        }

        protected override void OnCreated()
        {
            CoreEvents.CameraCreated?.Invoke(this);

            //D3DAPICall.CreateRenderTarget(PanelRegister.EditorMainScreen.Handle, this.Uid, 800, 600);
            //D3DAPICall.SetCameraProjection((float)(Math.PI / 2), 800.0f / 600.0f, 1.0f, 100.0f);
        }

        protected override void OnDestroyed()
        {

        }

        protected override void Update()
        {
            this.OwnerEntity.Transform.CalculateForwardAndUp();
            D3DAPICall.SetCameraTransform(
                    this.OwnerEntity.Transform.Translation,
                    this.OwnerEntity.Transform.m_forward,
                    this.OwnerEntity.Transform.m_up);
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
        private  FileLoad m_file;
        [JsonIgnore]
        public FileLoad File
        { get { return m_file; } }

        private bool m_isLoaded;

        protected override void OnCreated()
        {
            m_isLoaded = false;
            if(m_file == null)
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
            base.OnDestroyed();

            D3DAPICall.DeleteModelInstance(OwnerEntity.Uid);
        }

        private void SetPMDModel()
        {
            if(m_isLoaded)
            {
                //remove currently model


                m_isLoaded = false;
            }


            var result = D3DAPICall.SetPMDModel(OwnerEntity.Uid, m_file.FullPath);
            if(result == 1)
            {
                m_isLoaded = true;
            }

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
        [JsonProperty]
        private FileLoad m_file;
        [JsonIgnore]
        public FileLoad File
        { get { return m_file; } }

        private bool m_isLoaded;

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


            var result = D3DAPICall.SetBasicModel(OwnerEntity.Uid, m_file.FullPath);
            if (result == 1)
            {
                m_isLoaded = true;
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
        private FileLoad m_file;
        [JsonIgnore]
        public FileLoad File
        { get { return m_file; } }

        private bool m_isLoaded;

        protected override void OnCreated()
        {
            m_isLoaded = false;
            m_file = new FileLoad();
            m_file.OnChenged += SetAnimation;
        }

        protected override void OnDestroyed()
        {
            base.OnDestroyed();

        }

        private void SetAnimation()
        {
            if (m_isLoaded)
            {
                //remove currently 


                m_isLoaded = false;
            }


            var result = D3DAPICall.LoadAnimation(OwnerEntity.Uid, m_file.FullPath);
            if (result == 1)
            {
                m_isLoaded = true;
            }

        }

        public void SetAnimation(string path)
        {
            m_file.FullPath = path;
        }

        protected override void Update() 
        {
            D3DAPICall.UpdateAnimation(OwnerEntity.Uid);
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

}
