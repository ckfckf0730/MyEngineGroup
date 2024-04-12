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

        internal Camera MainCamera;


        internal void Boot()
        {
            if(MainCamera != null)
            {
                D3DAPICall.CreateRenderTarget(EngineRunTime.Instance.MainHandle, MainCamera.Uid,
                    MainCamera.m_width, MainCamera.m_height);
            }

            CoreEvents.SceneRunned?.Invoke(this);
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

            m_entitySerialzes.Clear();
        }

    }
}
