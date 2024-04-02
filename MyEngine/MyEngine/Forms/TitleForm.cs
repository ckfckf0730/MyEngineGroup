using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CkfEngine.Forms
{
    public partial class TitleForm : Form
    {
        public TitleForm()
        {
            InitializeComponent();
        }

        private void buttonNew_Click(object sender, EventArgs e)
        {
            SaveFileDialog saveFileDialog = new SaveFileDialog();
            saveFileDialog.Title = "Select a File";
            if (saveFileDialog.ShowDialog() == DialogResult.OK)
            {
   


            }
        }

        private void buttonSelect_Click(object sender, EventArgs e)
        {

        }
    }
}
