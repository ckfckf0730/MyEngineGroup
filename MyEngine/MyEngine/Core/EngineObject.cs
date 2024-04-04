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

        private string m_name;
        private ulong m_uid;

        public string Name
        {
            protected set { m_name = value; }
            get { return m_name; }
        }
        public ulong Uid
        {
            get { return m_uid; }
        }

        public EngineObject()
        {
            m_uid = UID.GetUID();
        }
    }
}
