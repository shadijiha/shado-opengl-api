using System;
using System.Runtime.CompilerServices;
using Shado.math;
using System.Runtime.InteropServices;

namespace Shado {

    [StructLayout(LayoutKind.Explicit)]
    public struct EntityDesc {
        [FieldOffset(0)] public ulong id;
        [FieldOffset(8)] public IntPtr scene;
    }

    public class Entity {

        public ulong Id { get; private set; }
        public Scene Scene { get; private set; }

        public TransformComponent Transform => GetComponent<TransformComponent>();

        protected Entity() {
            Id = CreateEntity(this.GetType(), this);
            Scene = new Scene(GetActiveScene());
        }

        internal Entity(ulong id, IntPtr scene) {
            Id = id;
            Scene = new Scene(scene);
        }

        protected virtual void OnCreate() { }

        protected virtual void OnUpdate(float dt) { }

        protected virtual void OnDestroyed() { }


        public T AddComponent<T>() where T : Component, new()
        {
            if (HasComponent<T>())
                return GetComponent<T>();

            AddComponent_Native(Id, Scene.GetNative(), typeof(T));
            T component = new T();
            component.Entity = this;
            return component;
        }

        public T GetComponent<T>() where T : Component, new() {
            T component = new T();
            component.Entity = this;
            return component;
        }

        public bool HasComponent<T>() where T : Component {
            return HasComponent_Native(Id, Scene.GetNative(), typeof(T));
        }

        public void Destroy() {
            Destroy_Native(Id, Scene.GetNative());
        }

        public static Entity Create()
        {
            return new Entity();
        }

        public static implicit operator EntityDesc(Entity e) { 
            return new EntityDesc() {
               id = e.Id,
               scene = e.Scene.GetNative(),
            };
        }

        /// Internal calls!
        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern ulong CreateEntity(Type type, Entity entity);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern IntPtr GetActiveScene();

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void AddComponent_Native(ulong id, IntPtr scene, Type type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool HasComponent_Native(ulong id, IntPtr scene, Type type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Destroy_Native(ulong id, IntPtr scene);

    }
}