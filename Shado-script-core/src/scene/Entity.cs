using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;

namespace Shado
{
    public class Entity
    {
        private ulong id;
        private Entity? m_Parent;

        // IMPORTANT:
        // This constructor is used by the script engine to create entities
        // Do not remove it or modify it
        // NEVER modify otherwise bugs will be cause when invoking Entity.Create()
        protected Entity() {
            ID = 0;
        }

        internal Entity(ulong id) {
            ID = id;
        }

        public ulong ID {
            get { return id; }
            private set { id = value; }
        } // TODO: IF we have weird error. revert this to be --> public readonly ulong ID

        public Vector3 translation {
            get { return transform.position; }
            set { transform.position = value; }
        }

        public TransformComponent transform => GetComponent<TransformComponent>();

        public Entity? Parent {
            get {
                unsafe {
                    ulong parentID = InternalCalls.Entity_GetParent(ID);

                    if (m_Parent == null || m_Parent.ID != parentID)
                        m_Parent = InternalCalls.Scene_IsEntityValid(parentID) ? new Entity(parentID) : null;
                }

                return m_Parent;
            }

            set {
                unsafe {
                    InternalCalls.Entity_SetParent(ID, value != null ? value.ID : 0);
                }
            }
        }

        public Entity[] children {
            get {
                Entity[] children;

                unsafe {
                    using var childIDs = InternalCalls.Entity_GetChildren(ID);
                    children = new Entity[childIDs.Length];
                    for (int i = 0; i < childIDs.Length; i++) {
                        children[i] = new Entity(childIDs[i]);
                    }
                }

                return children;
            }
        }

        /// <summary>
        /// Tag component of the entity (and its caching)
        /// </summary>
        public string tag {
            get { return GetComponent<TagComponent>().tag; }
            set { GetComponent<TagComponent>().tag = value; }
        }

        protected virtual void OnCreate() { }
        protected virtual void OnUpdate(float ts) { }
        protected virtual void OnLateUpdate(float ts) { }
        protected virtual void OnPhysicsUpdate(float ts) { }
        protected virtual void OnDestroy() { }

        public bool HasComponent<T>() where T : Component {
            unsafe {
                return InternalCalls.Entity_HasComponent(ID, typeof(T));
            }
        }

        public T? GetComponent<T>() where T : Component, new() {
            Type componentType = typeof(T);

            if (!HasComponent<T>()) {
                return null;
            }

            var component = new T { Entity = this };
            return component;
        }

        public bool RemoveComponent<T>() where T : Component {
            Type componentType = typeof(T);
            bool removed;

            unsafe {
                removed = InternalCalls.Entity_RemoveComponent(ID, componentType);
            }

            return removed;
        }

        public T? AddComponent<T>() where T : Component, new() {
            if (HasComponent<T>())
                return GetComponent<T>();

            unsafe {
                InternalCalls.Entity_CreateComponent(ID, typeof(T));
            }

            var component = new T { Entity = this };
            return component;
        }


        //public void Destroy() => Scene.DestroyEntity(this);

        /// <summary>
        /// Checks if this entity is a script entity of type T
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns>True if this entity is a script entity of type T</returns>
        public bool Is<T>() where T : Entity {
            var sc = GetComponent<ScriptComponent>();
            if (sc == null)
                return false;

            if (sc.Instance.Get() == null)
                return false;

            return sc.Instance.Get() is T;
        }

        /// <summary>
        /// Returns the script instance as type T if this entity is of the given type, otherwise null
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <returns></returns>
        public T? As<T>() where T : Entity {
            var sc = GetComponent<ScriptComponent>();
            return sc?.Instance.Get() as T;
        }

        // public static Entity FindEntityByName(string name) {
        //     ulong entityID = InternalCalls.Entity_FindEntityByName(name);
        //     if (entityID == 0)
        //         return null;
        //
        //     return new Entity(entityID);
        // }

        public static Entity? FindEntityByName(string name) {
            unsafe {
                ulong entityID = InternalCalls.Entity_FindEntityByName(name);
                if (entityID == 0)
                    return null;

                return new Entity(entityID);
            }
        }

        public bool Equals(Entity? other) {
            if (other is null)
                return false;

            if (ReferenceEquals(this, other))
                return true;

            return ID == other.ID;
        }

        private static bool IsValid(Entity? entity) {
            if (entity is null)
                return false;

            unsafe {
                return InternalCalls.Scene_IsEntityValid(entity.ID);
            }
        }

        public override int GetHashCode() => (int)ID;

        public static bool operator ==(Entity? entityA, Entity? entityB) =>
            entityA is null ? entityB is null : entityA.Equals(entityB);

        public static bool operator !=(Entity? entityA, Entity? entityB) => !(entityA == entityB);

        public static implicit operator bool(Entity entity) => IsValid(entity);
    }
}