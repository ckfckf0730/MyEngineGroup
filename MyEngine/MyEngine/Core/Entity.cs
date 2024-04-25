using CkfEngine.Core;
using CkfEngine.Editor;
using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine.Core
{
    [Serializable]
    public class Entity : EngineObject
    {
        internal Entity(string name) { Name = name; }

        #region StaticFunction

        private static Dictionary<ulong,Entity> m_entityTable = new Dictionary<ulong, Entity>();

        public static Entity[] GetAllEntities()
        {
            return m_entityTable.Values.ToArray();
        }

        internal static void CloseScene()
        {
            foreach (var entity in m_entityTable.Values)
            {
                entity.Released();
            }

            m_entityTable.Clear();
        }

        internal static void ResetParentByUid(List<Entity> entities)
        {
            Dictionary<ulong,Transform> transMap = new Dictionary<ulong,Transform>();


            foreach(var entity in entities)
            {
                var trans = entity.Transform;
                transMap.Add(trans.Uid, trans);
            }

            foreach(var trans in  transMap.Values)
            {
                if(trans.m_parentUid ==0)
                {
                    trans.SetParent(null);
                }
                else
                {
                    trans.SetParent(transMap[trans.m_parentUid]);
                }
            }
        }

        internal static void InitScene(Scene scene)
        {
            foreach(var entity in scene.m_entities)
            {
                InstantiateEntityBySceneData(entity);
            }
            ResetParentByUid(m_entityTable.Values.ToList());
        }

        private static void InstantiateEntityBySceneData(Entity obj)
        {
            //obj.ReacquireUID();
            m_entityTable.Add(obj.Uid, obj);
        }

        public static Entity CreateEntity(string name = null, Entity parent = null)
        {
            if (string.IsNullOrEmpty(name))
            {
                name = "New Entity";
            }

            var entity = new Entity(name);
            
            entity.m_transform = entity.CreateComponent<Transform>();
            m_entityTable.Add(entity.Uid, entity);
            entity.m_transform.SetParent(parent?.m_transform);

            CoreEvents.EntityCreated?.Invoke(entity);

            return entity;
        }

        public static void DeleteEntity(ulong uid)
        {
            Entity entity = null;
            m_entityTable.TryGetValue(uid, out entity);
            if(entity != null)
            {
                entity.Released();
                m_entityTable.Remove(uid);
            }
        }
        public static void DeleteEntity(Entity entity)
        {
            DeleteEntity(entity.Uid);
        }

        public static Entity FindEntity(ulong UID)
        {
            Entity entity = null;
            m_entityTable.TryGetValue(UID, out entity);
            return entity;
        }

        #endregion

        private Entity() { }

        private void Released()
        {
            foreach(var component in m_components.Values)
            {
                component.Release();
            }
            m_components.Clear();
        }

        [JsonIgnore]
        internal Dictionary<Type, Component> m_components = new Dictionary<Type, Component>();
        [JsonIgnore]
        public Component[] Components
        {
            get { return m_components.Values.ToArray(); }
        }

        [JsonProperty]
        private Transform m_transform;
        [JsonIgnore]
        public Transform Transform { get { return m_transform; } }

        internal void ResetTransform(Transform  trans)
        {
            m_transform = trans;
        }

        public T CreateComponent<T>() where T : Component, new()
        {
            Type type = typeof(T);
            if(m_components.TryGetValue(type, out Component component))
            {
                Console.WriteLine(Name + " already exist component: " + type.Name);
                return component as T;
            }

            T t = new T();
            t.BindEntity(this);
            m_components.Add(type, t);
            return t;
        }

        public Component CreateComponent(Type type)
        {
            if (m_components.TryGetValue(type, out Component component))
            {
                Console.WriteLine(Name + " already exist component: " + type.Name);
                return component;
            }
            Type baseType = typeof(Component);
            if (type!= baseType && baseType.IsAssignableFrom(type))
            {
                Component t = Activator.CreateInstance(type) as Component;
                t.BindEntity(this);
                m_components.Add(type, t);
                return t;
            }
            Console.WriteLine(type.Name + " isn't a correct component: ");
            return null;
        }

        public T GetComponent<T>() where T : Component
        {
            Component component;
            m_components.TryGetValue(typeof(T), out component);
            return component as T;
        }

        public Component GetComponent(Type type)
        {
            Component component;
            m_components.TryGetValue(type, out component);
            return component;
        }


        

    }

    [Serializable]
    internal class EntitySerialize
    {
        public EntitySerialize() { }

        [JsonProperty]
        internal Entity Obj;

        [JsonProperty]
        internal Dictionary<string, string> SerializedComponents;

        internal EntitySerialize(Entity obj)
        { 
            Obj = obj;
            SerializedComponents = new Dictionary<string, string>();
            foreach (var pair in Obj.m_components)
            {
                var valueJson = JsonConvert.SerializeObject(pair.Value);
                SerializedComponents.Add(pair.Key.FullName, valueJson);
            }
        }

        internal Entity Deserialize()
        {
            Obj.m_components.Clear();

            foreach(var pair in SerializedComponents)
            {
                Type type = Type.GetType(pair.Key);
                if(type == null)
                {
                    ScriptCompilate.ScriptTable.TryGetValue(pair.Key, out type);
                    if(type == null)
                    {
                        Console.WriteLine("Error! Can't Deserialize Type: " + pair.Key);
                        continue;
                    }
                }

                Component component =  JsonConvert.DeserializeObject(pair.Value, type) as Component;
                component.BindEntity(Obj);
                Obj.m_components.Add(type, component);

                if(component is Transform)
                {
                    Obj.ResetTransform(component as Transform);
                }
            }

            return Obj;
        }
    }
}
