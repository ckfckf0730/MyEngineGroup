using System;
using System.Collections.Generic;
using System.IO;
using System.IO.Pipes;
using System.Linq;
using System.Runtime.Serialization;
using System.Runtime.Serialization.Formatters.Binary;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    internal class Prefab 
    {
        [Serializable]
        private class SaveObj
        {
            public string name;
            public string typeName;
            public ulong uid;
            public ulong parentUid;
            public List<int> ints = new List<int>();
            public List<float> floats = new List<float>();
            public List<string> strings = new List<string>();
        }

        private Entity m_obj;
        private List<SaveObj> m_list = new List<SaveObj>();

        public void CreatePrefabFile(Entity obj, string path = "")
        {
            if(obj == null)
            {
                Console.WriteLine("CreatePrefabFile fault,The Entity is Null.");
                return;
            }

            FillList(obj);

            BinaryFormatter formatter = new BinaryFormatter();

            using (FileStream fileStream = new FileStream(path + obj.Name + ".prefab", FileMode.Create))
            {
                formatter.Serialize(fileStream, m_list);
            }


        }

        private void FillList(Entity obj)
        {
            if(obj != null) 
            {
                SaveObj saveEntity = new SaveObj();
                saveEntity.name = obj.Name;
                saveEntity.uid = obj.Uid;
                saveEntity.typeName =  obj.GetType().FullName;
                m_list.Add(saveEntity);

                var components = obj.Components;
                foreach(var component in components)
                {
                    SaveObj saveCom = new SaveObj();
                    saveCom.uid = component.Uid;
                    saveCom.typeName = component.GetType().FullName;

                    if(component is Transform)
                    {
                        Transform trans = (Transform) component;
                        if(trans.Parent!=null)
                        {
                            saveCom.parentUid = trans.Parent.Uid;
                        }
                        
                        saveCom.floats.Add(trans.Translation.X);
                        saveCom.floats.Add(trans.Translation.Y);
                        saveCom.floats.Add(trans.Translation.Z);

                        saveCom.floats.Add(trans.Rotation.X);
                        saveCom.floats.Add(trans.Rotation.Y);
                        saveCom.floats.Add(trans.Rotation.Z);

                        saveCom.floats.Add(trans.Scale.X);
                        saveCom.floats.Add(trans.Scale.Y);
                        saveCom.floats.Add(trans.Scale.Z);
                    }
                    else
                    {

                    }

                    m_list.Add(saveCom);
                }

                var children = obj.Transform.Children;
                if (children.Length>0)
                {
                    foreach(var child in children)
                    {
                        FillList(child.OwnerEntity);
                    }
                }
            }
        }

        private static Dictionary<ulong,Transform> s_mapRelat =new Dictionary<ulong, Transform>();
        public static Entity ReadPrefabFile(string path)
        {
            Entity entity = null;
            Entity rootEntity = null;
            BinaryFormatter formatter = new BinaryFormatter();

            List<SaveObj> deserializedObjectsList;
            if(!File.Exists(path))
            {
                Console.WriteLine("ReadPrefabFile fault, can't find file: " + path);
                return entity;
            }

            using (FileStream fileStream = new FileStream(path, FileMode.Open))
            {
                deserializedObjectsList = (List<SaveObj>)formatter.Deserialize(fileStream);

                if(deserializedObjectsList.Count>0)
                {
                    s_mapRelat.Clear();
                    entity = Entity.CreateEntity(deserializedObjectsList[0].name);
                    var curEntity = entity;
                    rootEntity = entity;

                    for (int i = 1; i < deserializedObjectsList.Count; i++) 
                    {
                        var obj = deserializedObjectsList[i];
                        Type type = Type.GetType(obj.typeName);
                        if(type == typeof(Entity))
                        {
                            entity = Entity.CreateEntity(obj.name);
                            curEntity = entity;
                        }
                        else if(type == typeof(Transform))
                        {
                            Transform transform = curEntity.Transform;
                            s_mapRelat.Add(obj.uid, transform);
                            if(obj.parentUid != 0)
                            {
                                transform.SetParent(s_mapRelat[obj.parentUid]);
                            }

                        }
                        else
                        {
                            curEntity.CreateComponent(type);
                        }
                    }

                }
                
            }

            return rootEntity;
        }
    }
}
