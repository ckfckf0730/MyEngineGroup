using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using CkfEngine.Core;
using System.Security.Cryptography;
using CkfEngine.Properties;
using System.Windows.Forms;
using System.Reflection;
using System.CodeDom.Compiler;
using System.Reflection.Emit;
using System.Threading;

namespace CkfEngine.Editor
{
    internal class ProjectManager
    {
        private ProjectManager() { }

        public static ProjectManager Instance
        {
            get
            {
                return instance != null ? instance : (instance = new ProjectManager());
            }
        }
        private static ProjectManager instance;

        internal CkfProject CurProject = null;
        internal Scene CurScene = null;
        internal string CurScenePath = null;

        internal void Init()
        {
            CoreEvents.EntityCreated += EntityOnCreated;
        }

        internal void EntityOnCreated(Entity entity)
        {
            CurScene?.m_entities .Add(entity);
        }

        internal void CreateNewProject(string fullPath)
        {
            string directory = Path.GetDirectoryName(fullPath);
            string proName = Path.GetFileName(fullPath);

            bool isExist = Directory.Exists(fullPath);
            if(isExist)
            {
                Console.WriteLine("Can't Create a new Prpject in existing Directory");
                return;
            }

            Directory.CreateDirectory(fullPath);

            string AssetsPath = fullPath + "/Assets";
            string LibraryPath = fullPath + "/Library";
            string prjFilePath = fullPath +"/" + proName+ ".cepj";

            Directory.CreateDirectory(AssetsPath);
            Directory.CreateDirectory(LibraryPath);
            File.Create(prjFilePath);

            CopyBasicAssets(AssetsPath);
            CreateLibrary(LibraryPath);


            ProjectVSBuild.CreateProject(fullPath, proName); 

            OpenProject(prjFilePath);
        }

        internal void CopyBasicAssets(string AssetsPath)
        {
            string engineAssetsPath = "Assets";
            CopyAllDirectory(engineAssetsPath, AssetsPath);
        }

        internal void CreateLibrary(string LibraryPath)
        {
            DynamicBuildEngineDll.BuildCore(LibraryPath);

            string engineLibraryPath = "Library";
            CopyAllDirectory(engineLibraryPath, LibraryPath);
        }

        internal void CopyAllDirectory(string scrPath, string descPath)
        {
            var dirs= Directory.GetDirectories(scrPath);
            var files = Directory.GetFiles(scrPath);

            foreach (var file in files)
            {
                string fileName = Path.GetFileName(file);
                string descFile = descPath + "/" + fileName;
                File.Copy(file, descFile);
            }

            foreach(var dir in dirs)
            {
                var lastFolderName = Path.GetFileName(dir);
                var newDir = descPath + "/" + lastFolderName;

                Directory.CreateDirectory(newDir);
                CopyAllDirectory(dir, newDir);
            }
        }


        internal void OpenProject(string path)
        {
            var dir = Path.GetDirectoryName(path);

            EditorEvents.OpenProject(dir);

            CurProject = new CkfProject();
            CurProject.Path = dir + "/";


            string projectName = Path.GetFileNameWithoutExtension(path); //test
            ProjectVSBuild.CreateProject(dir, projectName); //test
        }


        internal void SaveScene(Scene scene, string path)
        {
            var name = Path.GetFileNameWithoutExtension(path);
            scene.Name = name;

            scene.SerialzeEntities();
            string json = JsonConvert.SerializeObject(scene);

            File.WriteAllText(path, json);
        }


        internal void OpenScene(string path)
        {
            var jsonText = File.ReadAllText(path);

            var scene = JsonConvert.DeserializeObject<Scene>(jsonText);
            scene.ClearAndDeserialzeEntities();
            if (scene != null)
            {
                OpenScene(scene);
                CurScenePath = path;
            }
            else
            {
                Console.WriteLine("Can't Load Scene file: " + path);
            }
        }

        internal void OpenScene(Scene scene)
        {
            if (CurScene != scene)
            {
                CloseCurScene();
                CurScene = scene;
                InitScene();
            }
        }

        internal void CloseCurScene()
        {
            Entity.CloseScene();
        }

        private void InitScene()
        {
            Entity.InitScene(CurScene);
            EditorUI.Instance.CkfSceneItem.RefreshScene();
            foreach (var entity in Entity.GetAllEntities())
            {
                if (entity.Transform.Parent == null)
                {
                    entity.Transform.SetParent(null);
                    entity.Transform.EffectiveTransform();
                }
            }
        }

        private void ResetTransform(Transform trans)
        {
            trans.SetParent(trans.Parent);
            foreach(var child in trans.Children)
            {
                ResetTransform(child);
            }
        }

    }


    internal class CkfProject
    {
        private string m_path;
        internal string Path
        {
            set { m_path = value; }
            get { return m_path; }
        }

    }

}
