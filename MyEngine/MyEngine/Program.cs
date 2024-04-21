using CkfEngine.Core;
using CkfEngine.Editor;
using CkfEngine.Forms;
using Newtonsoft.Json.Linq;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CkfEngine
{
    internal static class Program
    {
        /// <summary>
        /// The main entry point for the application.
        /// </summary>
        [STAThread]
        static void Main()
        {
            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);

            Test();

            TitleForm titleForm = new TitleForm();
            Application.Run(titleForm);

            if(titleForm.IsOpenProject)
            {
                Application.Run(new Form1());
            }
        }


        static void Test()
        {
      


        }
    }

 
}
