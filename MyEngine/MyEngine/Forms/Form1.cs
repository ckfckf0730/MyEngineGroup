using CkfEngine.Editor;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Numerics;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace CkfEngine
{
    public partial class Form1 : Form
    {
        bool isD3dSuc = false;

        D3DAPICall d3dCall;

        private ContextMenuStrip SceneMenuStrip;

        public Form1()
        {
            InitializeComponent();

            d3dCall = new D3DAPICall();

            PanelRegister.EditorMainScreen = this.splitContainer3.Panel2;
            if (d3dCall.Init(this.splitContainer3.Panel2.Handle)> 0)
            {
                isD3dSuc = true;
            }

            InitEditor();

            //
            // timer1
            //
            this.timer1.Interval = 1000 / 20;
            this.timer1.Tick += this.Tick;
            this.timer1.Enabled = true;
            this.timer1.Start();
        }


        private CkfEditorSystem m_editorSystem;

        private TestScene testScene;

        private void InitEditor()
        {
            m_editorSystem = CkfEditorSystem.Instance;
            m_editorSystem.Init();

            //-----------Init UI module--------------
            this.Size = new Size(1500, 900);


            //-----------SceneItems-------
            SceneMenuStrip = new ContextMenuStrip();
            Editor.CkfEditorUI.Instance.Init(this);
            Editor.CkfEditorUI.Instance.CkfSceneItem.SetItemTree(this.SceneItemTree, this.SceneMenuStrip);
            //----------------------------

            //-----------Inspector--------
            FlowLayoutPanel panel = new FlowLayoutPanel();
            panel.FlowDirection = FlowDirection.LeftToRight;
            panel.AutoScroll = true;
            panel.Dock = DockStyle.Fill;
            panel.Size = new Size(panel.Size.Width, 2000);
            
            this.splitContainer1.Panel2.Controls.Add(panel);

            Editor.CkfEditorUI.Instance.CkfInspectorItem.SetControl(panel);
            //testScene = new TestScene();
            //testScene.Init();
            //----------------------------


            //---------------Assets------------------
            var assetsPanel = this.splitContainer2.Panel2;
            SplitContainer splitContainer = new SplitContainer();
            assetsPanel.Controls.Add(splitContainer);
            splitContainer.Dock  = DockStyle.Fill;
            

            Editor.CkfEditorUI.Instance.CkfAssetsUI.SetControl(splitContainer);
            //----------------------------
        }



        private void Tick(object sender, EventArgs e)
        {
            m_editorSystem.Update();
            //testScene.TestUpdate();

            if (isD3dSuc)
            {
                
                
            }
            StringBuilder msg = new StringBuilder(255);
            int isGet = 0;
            D3DAPICall.TryGetLog(ref isGet, msg);
            if (isGet != 0)
            {

                Console.WriteLine(msg);
            }
        }
    }

    public static class PanelRegister
    {
        public static Control EditorMainScreen;
    }
}
