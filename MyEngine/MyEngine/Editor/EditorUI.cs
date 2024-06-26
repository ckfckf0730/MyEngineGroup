using CkfEngine.Core;
using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace CkfEngine.Editor
{
    internal class EditorUI
    {
        private EditorUI() { }

        public static EditorUI Instance 
        { 
            get 
            {
                return instance != null ? instance : (instance = new EditorUI());
            }
        }
        private static EditorUI instance;

        private Form1 m_windowForm;
        internal Form1 MainForm
        {
            get { return m_windowForm; }
        }

        public void Init(Form1 form1)
        {
            m_windowForm = form1;
            m_ckfSceneItem = new CkfSceneItemUI();
            m_ckfInspectorItem = new InspectorUI();
            m_ckfAssetsUI = new AssetsUI();

            m_windowForm.Text = ProjectManager.Instance.CurProject.Path;
        }

        private CkfSceneItemUI m_ckfSceneItem;
        public CkfSceneItemUI CkfSceneItem { get {return m_ckfSceneItem; } }

        private InspectorUI m_ckfInspectorItem;
        public InspectorUI CkfInspectorItem { get { return m_ckfInspectorItem; } }

        private AssetsUI m_ckfAssetsUI;
        public AssetsUI CkfAssetsUI
        {
            get { return m_ckfAssetsUI; }
        }

    }

    abstract class CkfUIPanel
    {
        protected Control m_bindControl;
        public void SetControl(Control control)
        {
            m_bindControl = control;
            Init();
        }

        protected virtual void Init()
        {

        }
    }


}
