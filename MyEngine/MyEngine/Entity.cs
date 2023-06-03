using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine
{
    public static class UID
    {
        private static ulong uid = 0;
        public static ulong GetUID()
        {
            uid++;
            return uid;
        }
    }

    public class Entity : EntityBase
    {


        private Entity() { }

        private Entity(string name) { m_name = name; }

        private static Dictionary<ulong,Entity> m_entityTable = new Dictionary<ulong, Entity>();

        public static Entity CreateEntity(string name = null, Entity parent = null)
        {
            if (string.IsNullOrEmpty(name))
            {
                name = "New Entity";
            }

            var entity = new Entity(name);
            entity.m_uid = UID.GetUID();
            entity.m_transform = entity.CreateComponent<Transform>();
            entity.m_transform.SetParent(parent?.m_transform);
            m_entityTable.Add(entity.m_uid, entity);
            EventSetParent?.Invoke(entity.m_uid, parent == null ? 0 : parent.m_uid , 
                false, name);
            return entity;
        }

        public static Entity FindEntity(ulong UID)
        {
            Entity entity = null;
            m_entityTable.TryGetValue(UID, out entity);
            return entity;
        }


        private string m_name;
        private ulong m_uid;
        public ulong Uid { get { return m_uid; } }

        private Dictionary<Type, Component> m_components = new Dictionary<Type, Component>();
        public Component[] Components
        {
            get { return m_components.Values.ToArray(); }
        }

        private Transform m_transform;
        public Transform Transform { get { return m_transform; } }


        public T CreateComponent<T>() where T : Component, new()
        {
            Type type = typeof(T);
            if(m_components.TryGetValue(type, out Component component))
            {
                Console.WriteLine(m_name + " already exist component: " + type.Name);
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
                Console.WriteLine(m_name + " already exist component: " + type.Name);
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

        // entity UID, parent UID(null = 0), is delete
        internal static event Action<ulong, ulong, bool, string> EventSetParent;
        

    }

    public class EntityBase
    {
         
    }
}
