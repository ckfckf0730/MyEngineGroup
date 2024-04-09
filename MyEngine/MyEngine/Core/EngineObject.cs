using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    [Serializable]
    public class EngineObject
    {
        private static class UID
        {
            private static ulong uid = 0;
            public static ulong GetUID()
            {
                uid++;
                return uid;
            }
        }

        [JsonProperty]
        private string m_name;
        [JsonProperty]
        private ulong m_uid;

        [JsonIgnore]
        public string Name
        {
            set { m_name = value; }
            get { return m_name; }
        }

        [JsonIgnore]
        public ulong Uid
        {
            get { return m_uid; }
        }

        public EngineObject()
        {
            m_uid = UID.GetUID();
        }

        internal void ReacquireUID()
        {
            m_uid = UID.GetUID();
        }
    }
}
