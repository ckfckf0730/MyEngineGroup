using System;
using System.Collections.Generic;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Numerics;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine
{
    public abstract class Component
    {
        private Entity m_owner;
        public Entity OwnerEntity
        { get { return m_owner; } }

        protected virtual void OnCreated() { }

        protected virtual void OnDestroyed() { }

        protected virtual void Update() { }

        internal void BindEntity(Entity entity)
        {
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
        public Vector3 Scale 
        { 
            get { return m_scale; } 
            set { m_scale = value; EffectiveTransform(); }
        }


        private Transform m_parent;
        private Transform Parent { get { return m_parent; } }
        private List<Transform> m_children = new List<Transform>();

        internal void EffectiveTransform()
        {
            var worldMat = Matrix4x4.CreateScale(m_scale) *
                Matrix4x4.CreateFromYawPitchRoll(m_rotation.Y, m_rotation.X, m_rotation.Z) *
                Matrix4x4.CreateTranslation(m_translation);

            D3DAPICall.SetModelTransform(OwnerEntity.Uid, worldMat);
        }

        public void SetParent(Transform parent)
        {
            UnsetParent();
            if (parent != null && parent != this)
            {
                m_parent = parent;
                parent.m_children.Add(this);
            }
        }

        private void UnsetParent()
        {
            if(m_parent != null)
            {
                m_parent.m_children.Remove(this);
                m_parent = null;
            }
        }

    }

    public class Camera : Component
    {
        protected override void OnCreated()
        {

        }

        protected override void OnDestroyed()
        {

        }

        protected override void Update()
        {

        }
    }

    public class FileLoad
    {
        private string fullpath;
        public string FullPath
        {
            set { fullpath = value; OnChenged?.Invoke(); }
            get { return fullpath; }
        }
        public event Action OnChenged;
    }

    public class ModelBoneRenderer : Component
    {
        private  FileLoad m_file;
        public FileLoad File
        { get { return m_file; } }

        protected override void OnCreated()
        {
            m_file = new FileLoad();
            m_file.OnChenged += SetPMDModel;
        }

        private void SetPMDModel()
        {
            D3DAPICall.SetPMDModel(OwnerEntity.Uid, m_file.FullPath);
            OwnerEntity.Transform.EffectiveTransform();
        }

        public void SetPMDModel(string path)
        {
            m_file.FullPath = path;
        }


    }
}
