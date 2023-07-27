using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.CompilerServices;

namespace Shado
{
	public class Entity
	{
		protected Entity() { ID = 0; }

		internal Entity(ulong id)
		{
			ID = id;
		}

		public ulong ID { get; private set; }	// TODO: IF we have weird error. revert this to be --> public readonly ulong ID

		public Vector3 translation
		{
			get
			{
				InternalCalls.TransformComponent_GetTranslation(ID, out Vector3 result);
				return result;
			}
			set
			{
				InternalCalls.TransformComponent_SetTranslation(ID, ref value);
			}
		}

		public TransformComponent transform => GetComponent<TransformComponent>();

		public string tag {
			get { InternalCalls.TagComponent_GetTag(ID, out var tag); return tag; }
			set { InternalCalls.TagComponent_SetTag(ID, ref value); }
		}

		public bool HasComponent<T>() where T : Component, new()
		{
			Type componentType = typeof(T);
			return InternalCalls.Entity_HasComponent(ID, componentType);
		}

		public T GetComponent<T>() where T : Component, new()
		{
			if (!HasComponent<T>())
				return null;

			T component = new T() { Entity = this };
			return component;
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
        /// Creates a new entity. If you provide an object as pramater. An new entity will be created and the script 
        /// That is attached to it will be deplicated and attached to the new Entity.
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="obj">The source entity to copy. If null then a new entity 
        ///  without any components is created
        /// </param>
        /// <returns></returns>
        public T Create<T>(Entity obj = null) where T : Entity, new() {
			bool ignoreId = obj == null;

            ulong id = InternalCalls.Entity_Create(obj == null ? 0 : obj.ID, ref ignoreId);
            T e = new T
            {
                ID = id
            };
            return e;
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
