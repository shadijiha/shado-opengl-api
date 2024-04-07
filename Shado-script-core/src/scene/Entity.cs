using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;

namespace Shado
{
	public class Entity
	{
		private ulong id;

        // IMPORTANT:
        // This constructor is used by the script engine to create entities
        // Do not remove it or modify it
        // NEVER modify otherwise bugs will be cause when invoking Entity.Create()
        protected Entity() { ID = 0; }

		internal Entity(ulong id)
		{
			ID = id;
		}

		public ulong ID { get { return id; } private set { id = value; } }	// TODO: IF we have weird error. revert this to be --> public readonly ulong ID

		public Vector3 translation
		{
			get
			{
				return transform.position;
			}
			set
			{
				transform.position = value;
			}
		}

		/// <summary>
		/// Transform component of the entity (and its caching)
		/// </summary>
		private TransformComponent cachedTransform = null;
		public TransformComponent transform {
			get {
				if (cachedTransform is null)
                    cachedTransform = GetComponent<TransformComponent>();
                return cachedTransform;
			}
		}

		/// <summary>
		/// Tag component of the entity (and its caching)
		/// </summary>
		private TagComponent cachedTagComp = null;
		private string cachedTag = null;
		public string tag {
			get { 
				if (cachedTagComp is null)
                    cachedTagComp = GetComponent<TagComponent>();
				if (cachedTag is null)
					cachedTag = cachedTagComp.tag;
                return cachedTag;
			}
			set { 
				if (cachedTagComp is null)
                    cachedTagComp = GetComponent<TagComponent>();
				cachedTagComp.tag = value;
				cachedTag = null;	// Invalidate the cache
			}
		}

		public bool HasComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);
			return InternalCalls.Entity_HasComponent(ID, componentType);
		}

		public T GetComponent<T>() where T : Component, new()
		{
			// TODO: Check if T is an entity
			// TODO: test this
			if (typeof(T).IsSubclassOf(typeof(Entity)))
			{
				var script = GetComponent<ScriptComponent>();
				if (script is null)
					return null;

				return InternalCalls.GetScriptInstance(ID) as T;
			}
			else
			{
				if (!HasComponent<T>())
					return null;

				T component = new T() { Entity = this };
				return component;
			}
		}

		public void RemoveComponent<T>() where T : Component, new() {
			if (!HasComponent<T>())
				return;

			InternalCalls.Entity_RemoveComponent(ID, typeof(T));
		}

		public T AddComponent<T>() where T : Component, new()
		{
			if (HasComponent<T>())
				return GetComponent<T>();

			InternalCalls.Entity_AddComponent(ID, typeof(T));

			T component = new T() { Entity = this };
			return component;
		}


		public void Destroy(Entity entity) {
			InternalCalls.Entity_Destroy(entity.ID);
		}

        /// <summary>
        /// Creates a new entity
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="creator">The function that creates the entity</param>
        /// <returns>The created Entity</returns>
        public T Create<T>(Func<T> creator) where T : Entity {
			T entity = creator();
            ulong id = InternalCalls.Entity_CreateEntityId();
			entity.ID = id;
            InternalCalls.Entity_InvokeScriptEngineCreate(entity, id);
            return entity;
		}

        public IEnumerable<Component> GetAllComponents() {
	        return Component.GetAllComponentsTypes().Where(type => InternalCalls.Entity_HasComponent(ID, type))
		        .Select(component => Activator.CreateInstance(component))
		        .Cast<Component>()
		        .Select(component => {
			        component.Entity = this;
			        return component;
		        });
        }
        
		public static Entity FindEntityByName(string name)
		{
			ulong entityID = InternalCalls.Entity_FindEntityByName(name);
			if (entityID == 0)
				return null;

			return new Entity(entityID);
		}

		public static T FindEntityByName<T>(string name) where T : Entity, new() {
			return FindEntityByName(name).As<T>();
		}

		public T As<T>() where T : Entity, new()
		{
			object instance = InternalCalls.GetScriptInstance(ID);
			return instance as T;
		}
	}
}
