using System;
using System.IO;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using Newtonsoft.Json;
using CkfEngine.Core;


namespace CkfEngine.Editor
{
    internal static class ProjectManager
    {
        internal static CkfProject CurProject = null;

        internal static void CreateNewProject(string fullPath)
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
            CopyLibrary(LibraryPath);

            OpenProject(prjFilePath);
        }

        internal static void CopyBasicAssets(string AssetsPath)
        {
            string engineAssetsPath = "Assets";
            CopyAllDirectory(engineAssetsPath, AssetsPath);
        }

        internal static void CopyLibrary(string AssetsPath)
        {

        }

        internal static void CopyAllDirectory(string scrPath, string descPath)
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


        internal static void OpenProject(string path)
        {
            var dir = Path.GetDirectoryName(path);

            EditorEventManager.OpenProject(dir);

            CurProject = new CkfProject();
            CurProject.Path = dir + "/";


        }


        internal static void SaveScene(Scene scene, string path)
        {
            JsonSerializerSettings settings = new JsonSerializerSettings
            {
                ReferenceLoopHandling = ReferenceLoopHandling.Ignore // Ignore roop Handling
            };
            string json = JsonConvert.SerializeObject(scene, settings);

            File.WriteAllText(path, json);
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
