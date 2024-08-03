using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Xml.Linq;

namespace CkfEngine.Core
{
    [Serializable]
    internal class Scene
    {
        internal Scene() 
        {
            m_entities = new List<Entity>();
        }

        [JsonProperty]
        private string m_name;

        [JsonIgnore]
        public string Name
        {
            internal set { m_name = value; }
            get { return m_name; }
        }

        internal List<Entity> m_entities;

        [JsonProperty]
        internal List<EntitySerialize> m_entitySerialzes;

        [JsonProperty]
        private ulong m_uidCount;

        internal Camera MainCamera
        {
            set 
            { 
                m_mainCamera = value;
                m_mainCameraUid  = m_mainCamera == null ? 0 : m_mainCamera.Uid;
            }
            get { return m_mainCamera; }
        }
        private Camera m_mainCamera;
        [JsonProperty]
        private ulong m_mainCameraUid;

        internal void Boot()
        {
            if(MainCamera != null)
            {
                MainCamera.m_renderTargetHandle =
                    RenderTargetManager.Create(RuntimeControl.Instance.MainHandle,
                    MainCamera.m_width, MainCamera.m_height);
            }
        }

        internal void Stop()
        {
            if (MainCamera != null)
            {
                MainCamera.m_renderTargetHandle.Release();
            }
        }

        internal void SerialzeEntities()
        {
            m_entitySerialzes = new List<EntitySerialize>();

            foreach (Entity entity in m_entities)
            {
                m_entitySerialzes.Add(new EntitySerialize(entity));
            }
        }

        internal void ClearAndDeserialzeEntities()
        {
            m_entities.Clear();
            foreach (EntitySerialize item in m_entitySerialzes)
            {
                m_entities.Add(item.Deserialize());
            }

            if (m_mainCameraUid != 0)
            {
                Camera camera;
                Camera.CameraTable.TryGetValue(m_mainCameraUid, out camera);
                if(camera != null)
                {
                    m_mainCamera = camera;
                }
                else
                {
                    Console.WriteLine("Can't find Main Camera, UID: " + m_mainCamera);
                }
            }

            m_entitySerialzes.Clear();
        }

    }
}
