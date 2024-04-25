using CkfEngine.Editor;
using CkfEngine.Core;
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

            //----------main editor scene window-----------
            InitMainSceneWindow();

            //--------other editor panel------------
            InitEditor();

            //
            // timer1
            //
            this.timer1.Interval = 1000 / 20;
            this.timer1.Tick += this.Tick;
            this.timer1.Enabled = true;
            this.timer1.Start();
            //
            // timer2
            //
            this.timer2.Interval = 1000 / 20;
            this.timer2.Enabled = true;
            this.timer2.Start();


            this.timer2.Tick += RuntimeControl.Instance.Update;
        }


        private EditorSystem m_editorSystem;

        //private TestScene testScene;

        private void InitMainSceneWindow()
        {
            PanelRegister.EditorMainScreen = this.splitContainer3.Panel2;
            if (d3dCall.Init(this.splitContainer3.Panel2.Handle) > 0)
            {
                isD3dSuc = true;
            }
        }

        private void InitEditor()
        {
            m_editorSystem = EditorSystem.Instance;
            m_editorSystem.Init();

            //-----------Init UI module--------------
            this.Size = new Size(1500, 900);


            //-----------SceneItems-------
            SceneMenuStrip = new ContextMenuStrip();
            Editor.EditorUI.Instance.Init(this);
            Editor.EditorUI.Instance.CkfSceneItem.SetItemTree(this.SceneItemTree, this.SceneMenuStrip);
            //----------------------------

            //-----------Inspector--------
            FlowLayoutPanel panel = new FlowLayoutPanel();
            panel.FlowDirection = FlowDirection.LeftToRight;
            panel.AutoScroll = true;
            panel.Dock = DockStyle.Fill;
            panel.Size = new Size(panel.Size.Width, 2000);

            this.splitContainer1.Panel2.Controls.Add(panel);

            Editor.EditorUI.Instance.CkfInspectorItem.SetControl(panel);
            //testScene = new TestScene();
            //testScene.Init();
            //----------------------------


            //---------------Assets------------------
            var assetsPanel = this.splitContainer2.Panel2;
            SplitContainer splitContainer = new SplitContainer();
            assetsPanel.Controls.Add(splitContainer);
            splitContainer.Dock = DockStyle.Fill;


            Editor.EditorUI.Instance.CkfAssetsUI.SetControl(splitContainer);
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


        private void buttonSaveScene_Click(object sender, EventArgs e)
        {
            ProjectManager.Instance.SaveScene(ProjectManager.Instance.CurScene, ProjectManager.Instance.CurScenePath);
        }

        private void CompileButton_Click(object sender, EventArgs e)
        {
            ScriptCompilate.CompileAllScript();
        }

        private void buttonRunScene_Click(object sender, EventArgs e)
        {
            if(ProjectManager.Instance.CurScene == null)
            {
                Console.WriteLine("Error, no CurScene");
                return;
            }

            var runForm = new Form();
            int width = 800;
            int height = 600;
            runForm.Size = new Size(width, height);
            runForm.Show();
            RuntimeControl.Instance.SwitchState(runForm.Handle,ProjectManager.Instance.CurScene);
            runForm.FormClosed += (sender2, e2) =>
            {
                RuntimeControl.Instance.Stop();
            };

        }
    }

    internal static class PanelRegister
    {
        internal static Control EditorMainScreen;

        internal static Control GetExtendScreen()
        {
            Panel panel = new Panel();
            EditorMainScreen.Controls.Add(panel);
            ReplaceExtendScreen();

            return panel;
        }

        internal static void DeleteExtendScreen(Control panel)
        {
            EditorMainScreen.Controls.Remove(panel);
            ReplaceExtendScreen();
        }

        private static float ExtendScale = 0.25f;
        private static void ReplaceExtendScreen()
        {
            float width = EditorMainScreen.Width * ExtendScale;
            float height = EditorMainScreen.Height * ExtendScale;

            for (int i = 0; i < EditorMainScreen.Controls.Count; i++)
            {
                var panel = EditorMainScreen.Controls[i];
                panel.Size = new Size((int)width, (int)height);
                panel.Location = new Point((int)width * i, (int)0);
            }
        }

    }
}
