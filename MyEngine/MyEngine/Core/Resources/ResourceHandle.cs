using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal struct ResourceHandle
    {
        private UInt64 resPointer;
        private ResourceType resType;

        internal static ResourceHandle CreateHandle(UInt64 pointer, ResourceType type)
        {
            return new ResourceHandle() { resPointer = pointer, resType = type };
        }

        internal UInt64 GetPointer()
        {
            return resPointer;
        }

        internal ResourceType GetResourceType()
        {
            return resType;
        }

        internal void Release()
        {
            resPointer = 0;
            resType = ResourceType.None;
        }


        //private enum State
        //{
        //    NotCreated,
        //    Created,
        //    Deleted
        //}

        internal enum ResourceType
        {
            None,
            RenderTarget,
            Pipeline,
        }
    }




}
