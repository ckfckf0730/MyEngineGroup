using System;
using System.Collections.Generic;
using System.Data;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Numerics;
using System.Reflection;
using System.Security.AccessControl;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;
using CkfEngine.Core;
//using static System.Windows.Forms.VisualStyles.VisualStyleElement;

namespace CkfEngine.Editor
{
    internal class InspectorUI : CkfUIPanel
    {
        public Entity CurrentEntity;

        private Dictionary<string, ComponentUI> m_componentPanelTable;
        private System.Windows.Forms.Button m_button;
        private CkfAddComponentUI m_addComponentUI;

        private void OnEntitySelect(Entity entity)
        {
            if(CurrentEntity != entity)
            {
                CurrentEntity = entity;
                UpdatePanel();
            }
        }

        public void UpdatePanel()
        {
            HideAllPanel();

            if(CurrentEntity != null) 
            {
                foreach (var item in CurrentEntity.Components)
                {
                    UpdateOnePanel(item);
                }
            }
            
        }

        private void HideAllPanel()
        {
            foreach(var item in m_componentPanelTable.Values)
            {
                item.Hide();
            }
        }

        private void UpdateOnePanel(Component component)
        {
            var type = component.GetType();
            if (m_componentPanelTable.TryGetValue(type.Name,out var value))
            {
                value.UpdateData(component);
                return;
            }

            var newPanel = AddPanel();
            var newComponentUI = new ComponentUI();
            newComponentUI.Panel = newPanel;
            newComponentUI.Init(component);
            m_componentPanelTable.Add(type.Name, newComponentUI);
        }

        protected override void Init()
        {
            SelectUI.EventChangeSelect += OnEntitySelect;
            m_componentPanelTable = new Dictionary<string, ComponentUI>();
            m_addComponentUI = new CkfAddComponentUI();
            m_button = new System.Windows.Forms.Button();
            m_button.Name = "Add Component";
            m_button.Text = "Add Component";
            m_button.Size = new System.Drawing.Size(142, 23);
            m_button.TabIndex = 0;
            m_button.UseVisualStyleBackColor = true;
            m_button.Location = new System.Drawing.Point((m_bindControl.Width - m_button.Width) / 2, 10);
            m_addComponentUI.Init(m_button);
            m_bindControl.Controls.Add(m_button);
            ResetButton();
            //CreateTransformPanel();
        }

        private void ResetButton()
        {
            m_bindControl.Controls.Remove(m_button);
            m_bindControl.Controls.Add(m_button);    
        }

        private Control AddPanel()
        {
            var newPanel = new Panel();
            newPanel.BackColor = Color.FromArgb(180, 180, 200);

            m_bindControl.Controls.Add(newPanel);

            //newPanel.Dock = DockStyle.Fill;

 
            ResetButton();
            return newPanel;
        }

        private class ComponentUI
        {
            private string m_name;
            private List<FieldUI> m_fieldList = new List<FieldUI>();
            FieldInfo[] m_fieldInfos;

            private Control m_panel;
            public Control Panel
            {
                set { m_panel = value; }
                get { return m_panel; }
            }

            private int m_panelHeight;

            private Component m_component;
            //public Component Component
            //{
            //    get { return m_component; }
            //}

            public void Init(Component component)
            {
                m_component = component;
                Type type = component.GetType();
                m_fieldInfos = type.GetFields(
                    BindingFlags.Instance | BindingFlags.NonPublic | BindingFlags.Public);

                Label label = new Label();
                m_panel.Controls.Add(label);
                label.Text = type.Name;
                label.Location = new Point((m_panel.Width - label.Width) /2, 0);

                int top = 30;
                foreach (FieldInfo field in m_fieldInfos)
                {
                    var attribute = field.GetCustomAttribute<MyAttributeShowInspector>();
                    if (!field.IsPublic && attribute == null)
                    {
                        continue;
                    }
                    FieldUI fieldUI = new FieldUI();
                    fieldUI.CurComponent = component;
                    fieldUI.Init(field, m_panel, ref top);
                    
                    fieldUI.UpdateData();
                    m_fieldList.Add(fieldUI);
                    //}
                }
                m_panelHeight = top + 20;

                Panel.Height = m_panelHeight;
                Panel.Width = Panel.Parent.Width;
                //(m_panel.Parent as SplitContainer).SplitterDistance = m_panelHeight;

                Button deleteButton = new Button();
                m_panel.Controls.Add(deleteButton);
                deleteButton.Text = "Delete";
                deleteButton.Click += Delete;
                deleteButton.Location = new Point(Panel.Width - 70, 10);

            }

            private void Delete(object sender, EventArgs e)
            {
                m_component.OwnerEntity.DeleteComponent(m_component.GetType());
                m_component = null;
                Hide();
            }

            public void Hide()
            {
                //(m_panel.Parent as SplitContainer).SplitterDistance = 0;
                m_panel.Visible = false;
            }

            public void Show()
            {
                //(m_panel.Parent as SplitContainer).SplitterDistance = 0;
                m_panel.Visible = true;
            }

            public void UpdateData(Component component)
            {
                Show();
                m_component = component;

                foreach (var fieldUI in m_fieldList)
                {
                    fieldUI.CurComponent = component;
                    fieldUI.UpdateData();
                }
            }

            private class FieldUI
            {
                private Component m_curComponent;
                public Component CurComponent { set { m_curComponent = value; } }
                private FieldInfo m_info;
                private TextBox[] m_texts;

                public void Init(FieldInfo info,Control panel, ref int locationY)
                {
                    int offY = 25;
                    m_info = info;
                    Label label = new Label();
                    panel.Controls.Add(label);
                    label.Text = info.Name;
                    label.Location = new Point(0, locationY);
                    locationY += offY;

                    switch (info.FieldType.Name)
                    {
                        case "Single":
                            m_texts = new TextBox[1];
                            m_texts[0] = new TextBox();
                            m_texts[0].KeyPress += OnKeyPressOnlyFloat;
                            break;
                        case "Vector3":
                            m_texts = new TextBox[3];
                            m_texts[0] = new TextBox();
                            m_texts[0].KeyPress += OnKeyPressOnlyFloat;
                            m_texts[1] = new TextBox();
                            m_texts[1].KeyPress += OnKeyPressOnlyFloat;
                            m_texts[2] = new TextBox();
                            m_texts[2].KeyPress += OnKeyPressOnlyFloat;
                            break;
                        case "FileLoad":
                            m_texts = new TextBox[1];
                            m_texts[0] = new TextBox();
                            m_texts[0].Size = new Size(250,m_texts[0].Size.Height);
                            Button button = new Button();
                            button.Text = "Select File";
                            panel.Controls.Add(button);
                            button.Location = new Point(0, locationY);
                            locationY += offY;

                            var customAttribute = (MyAttributeLoadFileType)Attribute.GetCustomAttribute(info, typeof(MyAttributeLoadFileType));
                            var loadTypeStr = customAttribute?.Description;
                            button.Click += (sender, e) => 
                            {
                                OpenFileDialog openFileDialog = new OpenFileDialog();
                                openFileDialog.Title = "Select a File";
                                openFileDialog.Filter = loadTypeStr + " Files (*." + loadTypeStr + ")|*."+ loadTypeStr;
                                if (openFileDialog.ShowDialog() == DialogResult.OK)
                                {
                                    if (File.Exists(openFileDialog.FileName))
                                    {
                                        m_texts[0].Text = openFileDialog.FileName;
                                        FileLoad curLoad = (FileLoad)(m_info.GetValue(m_curComponent));
                                        curLoad.FullPath = openFileDialog.FileName;
                                    }

                                    
                                }
                            };

                            
                            break;
                    }
                   
                    if(m_texts != null)
                    {
                        int xCoord = 0;
                        foreach (var texbox in m_texts)
                        {
                            panel.Controls.Add(texbox);
                            texbox.Location = new Point(xCoord, locationY);
                            xCoord += 100;
                            texbox.TextChanged += OnValueChanged;
                        }
                        locationY += offY;
                    }
                    
                }


                public void UpdateData()
                {
                    switch (m_info.FieldType.Name)
                    {
                        case "Single":
                            float valueFloat = ((float)m_info.GetValue(m_curComponent));
                            m_texts[0].Text = valueFloat.ToString();
                            break;
                        case "Vector3":
                            Vector3 value = ((Vector3)m_info.GetValue(m_curComponent));
                            m_texts[0].Text = value.X.ToString();
                            m_texts[1].Text = value.Y.ToString();
                            m_texts[2].Text = value.Z.ToString();

                            break;
                        case "FileLoad":
                            FileLoad curLoad = (FileLoad)(m_info.GetValue(m_curComponent));
                            m_texts[0].Text = curLoad.FullPath;
                            curLoad.OnChenged += () =>
                            {
                                m_texts[0].Text = curLoad.FullPath;
                            };
                            break;
                    }
                }

                private void OnKeyPressOnlyFloat(object sender, KeyPressEventArgs e)
                {
                    if (!char.IsControl(e.KeyChar) && !char.IsDigit(e.KeyChar)
                        && (e.KeyChar != '.') && (e.KeyChar != '-'))
                    {
                        e.Handled = true;
                    }
                }

                private void OnValueChanged(object sender, EventArgs e)
                {
                    switch (m_info.FieldType.Name)
                    {
                        case "Single":
                            float value;
                            float.TryParse(m_texts[0].Text, out value);
                            m_info.SetValue(m_curComponent, value);
                            break;
                        case "Vector3":
                            float x,y,z;
                            float.TryParse(m_texts[0].Text,out x);
                            float.TryParse(m_texts[1].Text, out y);
                            float.TryParse(m_texts[2].Text, out z);
                            m_info.SetValue(m_curComponent, new Vector3(x,y,z));
                            (m_curComponent as Transform)?.EffectiveTransform();
                            break;
                        case "FileLoad":
                            break;
                    }
                }
            }
        }
    }

    internal class CkfAddComponentUI
    {
        private FormAddComponent m_form;
        private Dictionary<string,Type> m_componentTypeTable;

        public void Init(Button addButton)
        {
            SelectUI.EventChangeSelect += (entity) =>
            {
                if(entity != null)
                {
                    addButton.Show();
                }
                else
                {
                    addButton.Hide();
                }
            };
            if(SelectUI.CurEntity == null)
            {
                addButton.Hide();
            }
            else
            {
                addButton.Show();
            }


            m_componentTypeTable = new Dictionary<string,Type>();
            //m_form = new FormAddComponent();
            addButton.Click += Open;

            EditorEvents.ScriptsOnCompiled += UpdataComponentUI;
            ScriptCompilate.CompileAllScript();
        }

        private void UpdataComponentUI(Dictionary<string,Type> scriptTable)
        {
            Type baseType = typeof(Component);
            Assembly assembly = Assembly.GetExecutingAssembly(); // Or specify the assembly where your classes are defined
            var derivedTypes = new List<Type>();
            // Get all types in the assembly that are derived from the base type
            derivedTypes.AddRange(assembly.GetTypes()
            .Where(t => t != baseType && baseType.IsAssignableFrom(t)));

            foreach (Type script in scriptTable.Values)
            {
                derivedTypes.Add(script);
            }

            m_componentTypeTable.Clear();
            foreach (Type derivedType in derivedTypes)
            {
                if (derivedType.IsAbstract)
                {
                    continue;
                }

                m_componentTypeTable.Add(derivedType.Name, derivedType);
            }

            //CreateButtons();
        }

        private void CreateButtons()
        {
            Button templateButton = m_form.Controls[0] as Button;
            

            // Print the derived class names
            int locationX = templateButton.Location.X;
            int locationY = 20;
            foreach (Type derivedType in m_componentTypeTable.Values)
            {
                if (derivedType.IsAbstract)
                {
                    continue;
                }

                Button newButton = new Button();
                newButton.Name = derivedType.Name;
                newButton.Text = derivedType.Name;
                newButton.Size = templateButton.Size;
                newButton.Location = new Point(locationX, locationY);
                locationY += 25;
                m_form.Controls.Add(newButton);
                newButton.Click += OnClick;
            }
            m_form.Controls.Remove(templateButton);
        }

        private void OnClick(object sender, EventArgs e)
        {
            Type type;
            m_componentTypeTable.TryGetValue(
                (sender as Button).Name, out type);
            var componet = SelectUI.CurEntity.GetComponent(type);
            if(componet == null)
            {
                SelectUI.CurEntity.CreateComponent(type);
                EditorUI.Instance.CkfInspectorItem.UpdatePanel();
            }

            m_form.Close();
        }

        public void Open(object sender, EventArgs e)
        {
            if (m_form == null || m_form.IsDisposed) // Check if the form is null or disposed
            {
                m_form = new FormAddComponent();
                m_form.Owner = EditorUI.Instance.MainForm;
                CreateButtons();
            }
            m_form.Show();
        }

        public void Close()
        {
            m_form.Close();
        }
    }



    
    
}
