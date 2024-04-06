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

        public string Name
        {
            internal set { m_name = value; }
            get { return m_name; }
        }

        [JsonProperty]
        internal List<Entity> m_entities;




    }
}
