using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace Shado
{
	public abstract class Component
	{
		public Entity Entity { get; internal set; }
	}

	public class TagComponent : Component
	{
		public string tag
		{
			get
			{
				InternalCalls.TagComponent_GetTag(Entity.ID, out string tag);
				return tag;
			}
			set
			{
				InternalCalls.TagComponent_SetTag(Entity.ID, ref value);
			}
		}
	}

	public class TransformComponent : Component
	{
		public Vector3 position
		{
			get
			{
				InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
				return translation;
			}
			set
			{
				InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value);
			}
		}

		public Vector3 rotation
		{
			get
			{
				InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 translation);
				return translation;
			}
			set
			{
				InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value);
			}
		}

		public Vector3 scale
		{
			get
			{
				InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 translation);
				return translation;
			}
			set
			{
				InternalCalls.TransformComponent_SetScale(Entity.ID, ref value);
			}
		}
	}

	public class SpriteRendererComponent : Component
	{
		public Vector4 colour
		{
			get
			{
				Vector4 result;
				InternalCalls.SpriteRendererComponent_GetColour(Entity.ID, out result, GetType().Name);
				return result;
			}
			set
			{
				InternalCalls.SpriteRendererComponent_SetColour(Entity.ID, ref value, GetType().Name);
			}
		}

        public Texture2D texture
        {
            get
            {
                return InternalCalls.SpriteRendererComponent_GetTexture(Entity.ID, GetType().Name);
            }
            set
            {
				InternalCalls.SpriteRendererComponent_SetTexture(Entity.ID, value.native, GetType().Name);
            }
        }

        public float tilingFactor {
			get
			{
				float result;
				InternalCalls.SpriteRendererComponent_GetTilingFactor(Entity.ID, out result, GetType().Name);
				return result;
			}
			set
			{
				InternalCalls.SpriteRendererComponent_SetTilingFactor(Entity.ID, ref value, GetType().Name);
			}
		}
	}

	public class CircleRendererComponent : SpriteRendererComponent
	{
		public float thickness
		{
			get
			{
				float result;
				InternalCalls.CircleRendererComponent_GetFloatValue(Entity.ID, "thickness", out result);
				return result;
			}
			set
			{
				InternalCalls.CircleRendererComponent_SetFloatValue(Entity.ID, "thickness", ref value);
			}
		}

		public float fade
		{
			get
			{
				float result;
				InternalCalls.CircleRendererComponent_GetFloatValue(Entity.ID, "fade", out result);
				return result;
			}
			set
			{
				InternalCalls.CircleRendererComponent_SetFloatValue(Entity.ID, "fade", ref value);
			}
		}
	}

	public class RigidBody2DComponent : Component
	{
		public enum BodyType { Static = 0, Dynamic, Kinematic }

		public Vector2 linearVelocity
		{
			get
			{
				InternalCalls.Rigidbody2DComponent_GetLinearVelocity(Entity.ID, out Vector2 velocity);
				return velocity;
			}
		}

		public BodyType type
		{
			get => InternalCalls.Rigidbody2DComponent_GetType(Entity.ID);
			set => InternalCalls.Rigidbody2DComponent_SetType(Entity.ID, value);
		}

		public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake)
		{
			InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPosition, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, bool wake)
		{
			InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
		}

	}
}
