using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using CkfEngine.Core;

namespace CkfEngine.Editor
{

    internal class CkfSceneItemUI : CkfUIPanel
    {
        private System.Windows.Forms.TreeView m_itemTree;
        private ContextMenuStrip m_contextMenuStrip;

        private Dictionary<ulong, TreeNode> m_nodeTable = new Dictionary<ulong, TreeNode>();
        private TreeNode m_rootNode;

        public void SetItemTree(System.Windows.Forms.TreeView itemTree, ContextMenuStrip contextMenuStrip)
        {
            m_itemTree = itemTree;
            m_contextMenuStrip = contextMenuStrip; ;
            m_itemTree.NodeMouseClick += NodeMouseClick;
            m_rootNode = new TreeNode("None");
            m_itemTree.Nodes.Add(m_rootNode);
            m_itemTree.LabelEdit = true;
            m_itemTree.AfterLabelEdit += AfterLabelEdit;

            ToolStripMenuItem menuItem1 = new ToolStripMenuItem("Create Entity");
            menuItem1.Click += CreateEntityOnClicked;
            contextMenuStrip.Items.Add(menuItem1);

            ToolStripMenuItem menuItem2 = new ToolStripMenuItem("Create Prefab");
            menuItem2.Click += CreatePrefabOnClicked;
            contextMenuStrip.Items.Add(menuItem2);

            ToolStripMenuItem menuItem3 = new ToolStripMenuItem("Delete Entity");
            menuItem3.Click += DeleteEntityOnClicked;
            contextMenuStrip.Items.Add(menuItem3);

            ToolStripMenuItem menuItem4 = new ToolStripMenuItem("Rename");
            menuItem4.Click += RenameOnClicked;
            contextMenuStrip.Items.Add(menuItem4);

            Transform.EventSetParent += UpdateParentSetted;
        }

        private void AfterLabelEdit(object sender, NodeLabelEditEventArgs e)
        {
            TreeNode selectNode = m_itemTree.SelectedNode;
            if (selectNode.Tag is ulong)
            {
                ulong uid = (ulong)selectNode.Tag;
                Entity.FindEntity(uid).Name = e.Label;
                selectNode.Text = e.Label;
            }
        }

        internal void RefreshScene()
        {
            m_itemTree.Nodes.Clear();
            m_rootNode = new TreeNode(ProjectManager.Instance.CurScene.Name);
            m_itemTree.Nodes.Add(m_rootNode);
        }

        private void UpdateParentSetted(ulong entityID, ulong parentID, bool isDelete, string name)
        {
            TreeNode node;

            bool isFind = m_nodeTable.TryGetValue(entityID, out node);
            if (isDelete)
            {
                if (!isFind)
                {
                    return;
                }
                m_itemTree.Nodes.Remove(node);
                m_nodeTable.Remove(entityID);
                return;
            }

            if (!isFind)
            {
                node = new TreeNode(name);
                m_nodeTable.Add(entityID, node);
                node.Tag = entityID;
            }
            else
            {
                if (node.Parent != null)
                {
                    node.Parent.Nodes.Remove(node);
                }

            }

            if (parentID == 0)
            {
                m_rootNode.Nodes.Add(node);
            }
            else
            {
                TreeNode parentNode;
                if (m_nodeTable.TryGetValue(parentID, out parentNode))
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
            if (selectEntity != null)
            {
                Prefab prefab = new Prefab();
                prefab.CreatePrefabFile(selectEntity);
            }
        }

        void DeleteEntityOnClicked(object sender, EventArgs e)
        {
            TreeNode selectNode = m_itemTree.SelectedNode;
            if (selectNode.Tag is ulong)
            {
                ulong uid = (ulong)selectNode.Tag;
                Entity.DeleteEntity(uid);
            }

        }

        void RenameOnClicked(object sender, EventArgs e)
        {
            TreeNode selectNode = m_itemTree.SelectedNode;
            selectNode.BeginEdit();
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
                foreach (var node in m_nodeTable)
                {
                    if (node.Value == e.Node)
                    {
                        SelectUI.CurEntity = Entity.FindEntity(node.Key);
                        isFind = true;
                        break;
                    }
                }
                if (!isFind)
                {
                    SelectUI.CurEntity = null;
                }
            }
        }
    }

    internal static class SelectUI
    {
        public static event Action<Entity> EventChangeSelect;

        private static Entity s_curEntity;
        public static Entity CurEntity
        {
            get { return s_curEntity; }
            set 
            { 
                s_curEntity = value; 
                EventChangeSelect(s_curEntity); 
            }
        }
    }
}
