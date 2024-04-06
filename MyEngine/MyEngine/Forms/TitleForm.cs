using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using CkfEngine.Editor;

namespace CkfEngine.Forms
{
    public partial class TitleForm : Form
    {
        public TitleForm()
        {
            InitializeComponent();

            EditorEvents.OpenProject += ChangeEditorForm;
            IsOpenProject = false;
        }

        internal bool IsOpenProject;

        private void buttonNew_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.Title = "Create a new project";
            if (saveFileDialog.ShowDialog() == DialogResult.OK)
            {
                var proName = saveFileDialog.FileName;
                ProjectManager.CreateNewProject(proName);
                var fileName = Path.GetFileName(proName) + ".cepj";
                ProjectManager.OpenProject(proName + "/"+ fileName);
            }
        }

        private void buttonSelect_Click(object sender, EventArgs e)
        {
            OpenFileDialog openFileDialog = new OpenFileDialog();
            openFileDialog.Title = "Select a project";
            openFileDialog.Filter = "Files (*.cepj) | *.cepj" ;
            if (openFileDialog.ShowDialog() == DialogResult.OK)
            {
                var proName = openFileDialog.FileName;
                ProjectManager.OpenProject(proName);
            }
        }

        internal void ChangeEditorForm(string path)
        {
            IsOpenProject = true;
            this.Close();

        }
    }
}
