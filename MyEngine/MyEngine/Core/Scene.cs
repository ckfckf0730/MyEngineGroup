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
            var entity = new Entity("test1");

            entity.CreateComponent<Transform>();

            m_entities.Add(entity);
        }

        [JsonProperty]
        internal List<Entity> m_entities;




    }
}
