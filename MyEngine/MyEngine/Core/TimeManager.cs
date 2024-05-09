using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    public static class TimeManager
    {
        private static Stopwatch s_stopwatch;

        internal static void Init()
        {
            s_stopwatch = new Stopwatch();

            CoreEvents.SceneRunned += (scene) =>
            {
                s_stopwatch.Reset();
                s_stopwatch.Start();
            };
            CoreEvents.SceneStoped += (scene) =>
            {
                s_stopwatch.Stop();
                s_stopwatch.Reset();
            };
        }

        // ms
        public static double GetRunTime()
        {
            return s_stopwatch != null ? s_stopwatch.Elapsed.TotalMilliseconds : 0.0;
        }
    }
}
