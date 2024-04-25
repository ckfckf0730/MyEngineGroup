using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal class CoreEvents
    {
        internal static Action<Entity> EntityCreated;

        internal static Action<Camera> CameraCreated;
        internal static Action<Camera> CameraDestoried;

        internal static Action<Scene> SceneRunned;
        internal static Action<Scene> SceneStoped;
    }
}
