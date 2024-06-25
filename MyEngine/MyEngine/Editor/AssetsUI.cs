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
    internal class AssetsUI : CkfUIPanel
    {
        private TreeView m_tree;

        private TreeNode m_root;

        private ListView m_fileListView;

        private string m_curDir;
        private TreeNode m_curNode;

        private ContextMenuStrip m_contextMenuStrip;

        protected override void Init()
        {
            base.Init();

            var splitContainer = m_bindControl as SplitContainer;
            splitContainer.SplitterDistance = 280;
            splitContainer.Panel2.BackColor = Color.Ivory;

            //-----------direction tree-------------
            m_tree = new TreeView();
            m_tree.ShowPlusMinus = true;
            m_tree.LabelEdit = true;
            m_tree.AfterLabelEdit += AfterLabelEdit;
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

            m_fileListView.DoubleClick += FileListOnDoubleClicked;


            //------------ContextMenuStrip----------------
            m_contextMenuStrip = new ContextMenuStrip();
            m_tree.NodeMouseClick += NodeMouseClick;

            //create menu
            ToolStripDropDownButton menuItem1 = new ToolStripDropDownButton("Create");
            ToolStripMenuItem subMenuItem1 = new ToolStripMenuItem("Scene");
            ToolStripMenuItem subMenuItem2 = new ToolStripMenuItem("Folder");
            subMenuItem1.Click += (sender, e) =>
            {
                string path = ProjectManager.Instance.CurProject.Path +  GetRelativePath(m_tree.SelectedNode) + "/";
                for(int i = 0; i<10000; i++)
                {
                    string fileName = "NewScene";
                    if(i > 0)
                    {
                        fileName += "_" + i.ToString();
                    }
                    fileName += ".ckf";
                    string fulName = path + fileName;
                    if(!File.Exists(fulName))
                    {
                        ProjectManager.Instance.SaveScene(new Core.Scene(), fulName);
                        UpdateFIleIcons();
                        break;
                    }
                    
                }
            };
            subMenuItem2.Click += (sender, e) =>
            {
                string path = ProjectManager.Instance.CurProject.Path + GetRelativePath(m_tree.SelectedNode) + "/";
                for (int i = 0; i < 10000; i++)
                {
                    string folderName = "NewFolder";
                    if (i > 0)
                    {
                        folderName += "_" + i.ToString();
                    }

                    string fulName = path + folderName;
                    if (!Directory.Exists(fulName))
                    {
                        Directory.CreateDirectory(fulName);
                        m_tree.SelectedNode.Nodes.Add(folderName);

                        break;
                    }

                }
            };
            ToolStripDropDownMenu subMenu = new ToolStripDropDownMenu();
            subMenu.Items.Add(subMenuItem1);
            subMenu.Items.Add(subMenuItem2);
            menuItem1.DropDown = subMenu;
            m_contextMenuStrip.Items.Add(menuItem1);

            // rename  menu 
            ToolStripMenuItem menuItem2 = new ToolStripMenuItem("Rename");
            menuItem2.Click += (sender, e) =>
            {
                m_tree.SelectedNode.BeginEdit();
            };
            m_contextMenuStrip.Items.Add(menuItem2);
        }

        private void AfterLabelEdit(object sender, NodeLabelEditEventArgs e)
        {
            if (e.CancelEdit) 
            {
                Console.WriteLine("Label edit cancelled.");
            }   
            else if (e.Label != null) 
            {
                TreeNode selectNode = m_tree.SelectedNode;
                int foldNameLen = selectNode.Text.Length;
                string oldPath = ProjectManager.Instance.CurProject.Path + GetRelativePath(m_tree.SelectedNode);
                string newPath = oldPath.Remove(oldPath.Length - foldNameLen) + e.Label;
                Directory.Move(oldPath, newPath);

                selectNode.Text = e.Label;

                e.CancelEdit = true;
            }
        }

        private void NodeMouseClick(object sender, TreeNodeMouseClickEventArgs e)
        {
            //right click: show menu (example: add entity, delete, ...)
            if (e.Button == MouseButtons.Right)
            {
                m_tree.SelectedNode = e.Node;

                m_contextMenuStrip.Show(m_tree, e.Location);
            }
            //select enetity
            else if (e.Button == MouseButtons.Left)
            {
                m_tree.SelectedNode = e.Node;

            }
        }

        private void NodeExpanded(object sender, TreeViewEventArgs e)
        {
            foreach (var node in e.Node.Nodes)
            {
                UpdateNodeUI(node as TreeNode);
            }
        }

        private void AfterSelect(object sender, TreeViewEventArgs e)
        {
            var tree = sender as TreeView;
            var path = GetRelativePath(tree.SelectedNode);

            if (m_curDir != path)
            {
                m_curDir = path;
                m_curNode = tree.SelectedNode;
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

            var fullPath = ProjectManager.Instance.CurProject.Path + m_curDir;
            var files = Directory.GetFiles(fullPath);

            foreach (string file in files)
            {
                FileInfo fileInfo = new FileInfo(file);
                ListViewItem item = new ListViewItem(fileInfo.Name);
                item.SubItems.Add(fileInfo.Length.ToString());
                item.SubItems.Add(fileInfo.LastWriteTime.ToString());
                item.ImageKey = fileInfo.Extension;
                m_fileListView.Items.Add(item);
            }

            m_fileListView.DoubleClick += ItemViewOnDoubleClicked;
        }

        private void FileListOnDoubleClicked(object sender, EventArgs e)
        {
            var fileName = (sender as ListView).FocusedItem.Text;
            var path = ProjectManager.Instance.CurProject.Path + GetRelativePath(m_tree.SelectedNode);
            var fullFIlePath = path + "/" + fileName;

            var fileType = Path.GetExtension(fileName).ToLower();
            
            switch(fileType)
            {
                case ".ckf":
                    ProjectManager.Instance.OpenScene(fullFIlePath);
                break;
                    default: 
                    
                    break;
            }

        }

        private void ItemViewOnDoubleClicked(object sender, EventArgs e)
        {
            if (m_fileListView.SelectedItems.Count > 0)
            {
                var file = m_fileListView.SelectedItems[0].Text;
                var dir = GetRelativePath(m_curNode);
                var fullPath = ProjectManager.Instance.CurProject.Path + dir + "/" + file;

                if (Path.GetExtension(file).ToLower() == "scene")
                {

                }
            }
        }


        private void UpdateNodeUI(TreeNode node)
        {
            var dirs = Directory.GetDirectories(ProjectManager.Instance.CurProject.Path + GetRelativePath(node));
            foreach (var dir in dirs)
            {
                node.Nodes.Add(new TreeNode(GetFinalDirName(dir)));
            }
        }

        private string GetRelativePath(TreeNode node)
        {
            if (node?.Text == null)
            {
                return null;
            }

            var curNode = node;
            string path = node.Text;
            while (curNode.Parent != null)
            {
                curNode = curNode.Parent;
                path = curNode.Text + '/' + path;

            }
            return path;
        }

        private string GetFinalDirName(string dir)
        {
            var strs = dir.Split('/', '\\');
            if (strs.Length == 0)
            {
                return null;
            }

            return strs[strs.Length - 1];
        }


    }
}
