using System;
using System.Collections.Generic;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using System.IO;
using static System.Net.WebRequestMethods;

namespace CkfEngine.Editor
{
    internal class CkfAssetsUI : CkfUIPanel
    {
        private TreeView m_tree;

        private TreeNode m_root;

        private ListView m_fileListView;

        private string m_curDir;


        protected override void Init()
        {
            base.Init();

            var splitContainer = m_bindControl as SplitContainer;
            splitContainer.SplitterDistance = 280;
            splitContainer.Panel2.BackColor = Color.Ivory;

            //-----------direction tree-------------
            m_tree = new TreeView();
            m_tree.ShowPlusMinus = true;
            m_root = new TreeNode();
            m_root.Text = "Assets";
            m_tree.Nodes.Add(m_root);
            m_tree.Dock = DockStyle.Fill;


            splitContainer.Panel1.Controls.Add(m_tree);

            UpdateNodeUI(m_root);

            m_tree.AfterExpand += NodeExpanded;
            m_tree.AfterSelect += AfterSelect;

            //----------file icon-------------
            m_fileListView = new ListView();
            m_fileListView.View = View.Details;
            splitContainer.Panel2.Controls.Add(m_fileListView);
            m_fileListView.Dock = DockStyle.Fill;

            m_fileListView.Columns.Add("Name", 200);
            m_fileListView.Columns.Add("Size", 100);
            m_fileListView.Columns.Add("Modified", 150);

            ImageList largeImageList = new ImageList();
            largeImageList.ImageSize = new System.Drawing.Size(32, 32);
            m_fileListView.LargeImageList = largeImageList;

            ImageList smallImageList = new ImageList();
            smallImageList.ImageSize = new System.Drawing.Size(16, 16);
            m_fileListView.SmallImageList = smallImageList;
        }

        private void NodeExpanded(object sender, TreeViewEventArgs e)
        {
            foreach(var node in e.Node.Nodes)
            {
                UpdateNodeUI(node as TreeNode);
            }
        }

        private void AfterSelect(object sender, TreeViewEventArgs e)
        {
            var tree = sender as TreeView;
            var path  =  GetFullPath(tree.SelectedNode);

            if(m_curDir!= path)
            {
                m_curDir = path;
                UpdateFIleIcons();

                Console.WriteLine(m_curDir);
            }

        }

        private void UpdateFIleIcons()
        {
            m_fileListView.Items.Clear();

            if (string.IsNullOrEmpty(m_curDir))
            {
                return;
            }

            var files = Directory.GetFiles(m_curDir);

            foreach (string file in files)
            {
                FileInfo fileInfo = new FileInfo(file);
                ListViewItem item = new ListViewItem(fileInfo.Name);
                item.SubItems.Add(fileInfo.Length.ToString());
                item.SubItems.Add(fileInfo.LastWriteTime.ToString());
                item.ImageKey = fileInfo.Extension; 
                m_fileListView.Items.Add(item);
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
            if(node?.Text == null)
            {
                return null;
            }

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
