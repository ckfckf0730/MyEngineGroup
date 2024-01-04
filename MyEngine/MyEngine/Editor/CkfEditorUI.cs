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
        }

        private CkfSceneItemUI m_ckfSceneItem;
        public CkfSceneItemUI CkfSceneItem { get {return m_ckfSceneItem; } }

        private CkfInspectorUI m_ckfInspectorItem;
        public CkfInspectorUI CkfInspectorItem { get { return m_ckfInspectorItem; } }

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

    internal class CkfSceneItemUI : CkfUIPanel
    {
        private System.Windows.Forms.TreeView m_itemTree;
        private ContextMenuStrip m_contextMenuStrip;

        private Dictionary<ulong,TreeNode> m_nodeTable = new Dictionary<ulong,TreeNode>();
        private TreeNode m_rootNode;

        public void SetItemTree(System.Windows.Forms.TreeView itemTree, ContextMenuStrip contextMenuStrip)
        {
            m_itemTree = itemTree;
            m_contextMenuStrip = contextMenuStrip; ;
            m_itemTree.NodeMouseClick += NodeMouseClick;
            m_rootNode = new TreeNode("Scene");
            m_itemTree.Nodes.Add(m_rootNode);

            ToolStripMenuItem menuItem1 = new ToolStripMenuItem("Create Entity");
            menuItem1.Click += CreateEntityOnClicked;
            contextMenuStrip.Items.Add(menuItem1);

            ToolStripMenuItem menuItem2 = new ToolStripMenuItem("Create Prefab");
            menuItem2.Click += CreatePrefabOnClicked;
            contextMenuStrip.Items.Add(menuItem2);

            Transform.EventSetParent += UpdateParentSetted;
        }

        private void UpdateParentSetted(ulong entityID, ulong parentID, bool isDelete, string name)
        {
            TreeNode node;

            bool isFind = m_nodeTable.TryGetValue(entityID, out node);
            if (isDelete)
            {
                if(!isFind)
                {
                    return;
                }
                m_itemTree.Nodes.Remove(node);
                m_nodeTable.Remove(entityID);
                return;
            }

            if(!isFind)
            {
                node = new TreeNode(name);
                m_nodeTable.Add(entityID, node);
                node.Tag = entityID;
            }
            else
            {
                if(node.Parent!= null)
                {
                    node.Parent.Nodes.Remove(node);
                }

            }

            if(parentID == 0)
            {
                m_rootNode.Nodes.Add(node);
            }
            else
            {
                TreeNode parentNode;
                if(m_nodeTable.TryGetValue(parentID, out parentNode))
                {
                    parentNode.Nodes.Add(node);
                }
                else
                {
                    m_rootNode.Nodes.Add(node);
                }
            }

        }

        void CreateEntityOnClicked(object sender, EventArgs e)
        {
            TreeNode selectNode = m_itemTree.SelectedNode;
            Entity selectEntity = null;
            if (selectNode.Tag is ulong)
            {
                ulong uid = (ulong)selectNode.Tag;
                selectEntity = Entity.FindEntity(uid); 
            }
            var newEntity = Entity.CreateEntity("NewEntity", selectEntity);
        }

        void CreatePrefabOnClicked(object sender, EventArgs e)
        {
            TreeNode selectNode = m_itemTree.SelectedNode;
            Entity selectEntity = null;
            if (selectNode.Tag is ulong)
            {
                ulong uid = (ulong)selectNode.Tag;
                selectEntity = Entity.FindEntity(uid);
            }
            if(selectEntity !=null)
            {
                Prefab prefab = new Prefab();
                prefab.CreatePrefabFile(selectEntity);
            }
        }

        private void NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            //right click: show menu (example: add entity, delete, ...)
            if (e.Button == MouseButtons.Right)
            {
                m_itemTree.SelectedNode = e.Node;

                m_contextMenuStrip.Show(m_itemTree, e.Location);
            }
            //select enetity
            else if (e.Button == MouseButtons.Left)
            {
                m_itemTree.SelectedNode = e.Node;
                bool isFind = false;
                foreach(var  node in m_nodeTable) 
                {
                    if(node.Value == e.Node)
                    {
                        CkfSelectUI.CurEntity = Entity.FindEntity(node.Key);
                        isFind = true;
                        break;
                    }
                }
                if(!isFind)
                {
                    CkfSelectUI.CurEntity = null;
                }
            }
        }
    }

    internal static class CkfSelectUI
    {
        public static event Action<Entity> EventChangeSelect;

        private static Entity s_curEntity;
        public static Entity CurEntity
        {
            get { return s_curEntity; }
            set { s_curEntity = value; EventChangeSelect(s_curEntity); }
        }
    }

}
