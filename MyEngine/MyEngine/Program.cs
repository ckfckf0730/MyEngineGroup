using CkfEngine.Editor;
using CkfEngine.Forms;
using System;
using System.Collections.Generic;
using System.Linq;
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

            TitleForm titleForm = new TitleForm();
            Application.Run(titleForm);

            if(titleForm.IsOpenProject)
            {
                Application.Run(new Form1());
            }

        }
    }
}
