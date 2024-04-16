using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal class ScriptTest : Behaviour
    {
        protected override void OnCreated()
        {
            
        }

        protected override void OnDestroyed() 
        { 

        }

        private float offset;
        public float speed = 0.01f;

        protected override void Update()
        {
            offset += speed;
            if(offset > 5)
            {
                offset = -5;
            }
            OwnerEntity.Transform.Translation = new System.Numerics.Vector3 (offset, 0, 0);
        }
    }
}
