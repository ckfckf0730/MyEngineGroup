using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;

namespace CkfEngine.Editor
{
    internal class CkfAssetsUI : CkfUIPanel
    {
        private TreeView m_tree;

        private TreeNode m_root;


        protected override void Init()
        {
            base.Init();

            var splitContainer = m_bindControl as SplitContainer;
            splitContainer.SplitterDistance = 280;
            splitContainer.Panel2.BackColor = Color.Red;

            m_tree = new TreeView();
            m_tree.ShowPlusMinus = true;
            m_root = new TreeNode();
            m_root.Text = "Assets";
            m_tree.Nodes.Add(m_root);
                
            splitContainer.Panel1.Controls.Add(m_tree);

            UpdateNodeUI(m_root);

            m_tree.AfterExpand += NodeExpanded;
        }

        private void NodeExpanded(object sender, TreeViewEventArgs e)
        {
            foreach(var node in e.Node.Nodes)
            {
                UpdateNodeUI(node as TreeNode);
            }
        }

        private void UpdateNodeUI(TreeNode node)
        {
            var dirs = Directory.GetDirectories(GetFullPath(node));
            foreach (var dir in dirs)
            {
                node.Nodes.Add(new TreeNode(GetFinalDirName(dir)));
            }
        }

        private string GetFullPath(TreeNode node)
        {
            var curNode = node;
            string path = node.Text;
            while(curNode.Parent != null) 
            {
                curNode = curNode.Parent;
                path = curNode.Text + '/'+ path;

            }
            return path;
        }

        private string GetFinalDirName(string dir)
        {
            var strs = dir.Split('/','\\');
            if (strs.Length == 0)
            { 
                return null;
            }

            return strs[strs.Length - 1];
        }
    }
}
