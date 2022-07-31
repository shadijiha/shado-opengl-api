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

        // Shortcut functions
        public TransformComponent Transform => GetComponent<TransformComponent>();

        public string Tag {
            get { return GetComponent<TagComponent>().Tag; }
            set { GetComponent<TagComponent>().Tag = value; }
        }

        // END shortcut functions
        protected Entity() {
            // Keep this in comment otherwise 1 extra unwanted entity
            // will be created each time [C++] onRuntimeStart() is called

            //Id = CreateEntity(this.GetType(), this);
            //Scene = new Scene(GetActiveScene());
        }

        internal Entity(ulong id, IntPtr scene) {
            Id = id;
            Scene = new Scene(scene);
        }

        protected virtual void OnCreate() { }

        protected virtual void OnUpdate(float dt) { }

        protected virtual void OnDestroyed() { }

        protected virtual void OnCollision2DEnter(Collision2DInfo info, Entity other) { }

        protected virtual void OnCollision2DLeave(Collision2DInfo info, Entity other) { }

        public T AddComponent<T>() where T : Component, new()
        {
            Validate();

            if (HasComponent<T>())
                return GetComponent<T>();

            AddComponent_Native(Id, Scene.GetNative(), typeof(T));
            T component = new T();
            component.Entity = this;
            return component;
        }

        public T GetComponent<T>() where T : Component, new() {
            Validate();

            T component = new T();
            component.Entity = this;
            return component;
        }

        public bool HasComponent<T>() where T : Component {
            Validate();

            return HasComponent_Native(Id, Scene.GetNative(), typeof(T));
        }

        public void RemoveComponent<T>() where T : Component, new()
        {
            Validate();

            RemoveComponent_Native(Id, Scene.GetNative(), typeof(T));
        }

        public void Destroy() {
            OnDestroyed();
            Destroy_Native(Id, Scene.GetNative());
            //Scene = Scene.Null;
        }

        public bool IsValid() {
            return IsValid_Native(Id, Scene.GetNative());
        }

        public static Entity Create()
        {
            Entity e = new Entity();
            Scene scene = new Scene(GetActiveScene());
            e.Scene = scene;

            ulong id = CreateEntity(typeof(Entity), e);
            e.Id = id; 
            return e;
        }

        public static implicit operator EntityDesc(Entity e) { 
            return new EntityDesc() {
               id = e.Id,
               scene = e.Scene.GetNative(),
            };
        }

        private void Validate() {
            if (!IsValid())
                throw new InvalidOperationException("Entity is invalid");
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
        private static extern void RemoveComponent_Native(ulong id, IntPtr scene, Type type);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool Destroy_Native(ulong id, IntPtr scene);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern bool IsValid_Native(ulong id, IntPtr scene);
    }
}