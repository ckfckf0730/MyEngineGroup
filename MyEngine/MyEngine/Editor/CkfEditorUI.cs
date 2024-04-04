using CkfEngine.Core;
using System;
using System.Collections.Generic;
using System.Windows.Forms;

namespace CkfEngine.Editor
{
    internal class CkfEditorUI
    {
        private CkfEditorUI() { }

        public static CkfEditorUI Instance 
        { 
            get 
            {
                return instance != null ? instance : (instance = new CkfEditorUI());
            }
        }
        private static CkfEditorUI instance;

        private Form1 m_windowForm;
        public void Init(Form1 form1)
        {
            m_windowForm = form1;
            m_ckfSceneItem = new CkfSceneItemUI();
            m_ckfInspectorItem = new CkfInspectorUI();
            m_ckfAssetsUI = new CkfAssetsUI();
        }

        private CkfSceneItemUI m_ckfSceneItem;
        public CkfSceneItemUI CkfSceneItem { get {return m_ckfSceneItem; } }

        private CkfInspectorUI m_ckfInspectorItem;
        public CkfInspectorUI CkfInspectorItem { get { return m_ckfInspectorItem; } }

        private CkfAssetsUI m_ckfAssetsUI;
        public CkfAssetsUI CkfAssetsUI
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
