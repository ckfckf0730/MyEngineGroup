using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    [Serializable]
    internal class Scene
    {
        [JsonProperty]
        internal List<Entity> m_entities;

        [JsonProperty]
        internal List<int> list;
    }
}
