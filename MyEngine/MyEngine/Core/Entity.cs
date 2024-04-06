﻿using CkfEngine.Core;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace CkfEngine
{
    [Serializable]
    public class Entity : EngineObject
    {
        internal Entity(string name) { Name = name; }

        #region EntityTable

        private static Dictionary<ulong,Entity> m_entityTable = new Dictionary<ulong, Entity>();

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

}
