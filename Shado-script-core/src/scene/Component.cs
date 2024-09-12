using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;

namespace Shado
{
	/// <summary>
	/// Base class for all components
	/// </summary>
	public abstract class Component
	{
		public Entity Entity { get; internal set; }

		internal static IEnumerable<Type> GetAllComponentsTypes() {
			return Assembly.GetExecutingAssembly().GetTypes()
				.Where(type => type.IsSubclassOf(typeof(Component)) && !type.IsAbstract);
		}
	}

	public class TagComponent : Component
	{
		public string tag {
			get {
				InternalCalls.TagComponent_GetTag(Entity.ID, out string tag);
				return tag;
			}
			set { InternalCalls.TagComponent_SetTag(Entity.ID, ref value); }
		}
	}

	public class TransformComponent : Component
	{
		public Vector3 position {
			get {
				InternalCalls.TransformComponent_GetTranslation(Entity.ID, out Vector3 translation);
				return translation;
			}
			set { InternalCalls.TransformComponent_SetTranslation(Entity.ID, ref value); }
		}

		public Vector3 rotation {
			get {
				InternalCalls.TransformComponent_GetRotation(Entity.ID, out Vector3 translation);
				return translation;
			}
			set { InternalCalls.TransformComponent_SetRotation(Entity.ID, ref value); }
		}

		public Vector3 scale {
			get {
				InternalCalls.TransformComponent_GetScale(Entity.ID, out Vector3 translation);
				return translation;
			}
			set { InternalCalls.TransformComponent_SetScale(Entity.ID, ref value); }
		}

		public Entity parent {
			get {
				ulong parentId = InternalCalls.TransformComponent_GetParentId(Entity.ID);
				return parentId == 0 ? null : new Entity(parentId);
			}
			set { InternalCalls.TransformComponent_SetParentId(Entity.ID, value.ID); }
		}
	}

	public class SpriteRendererComponent : Component
	{
		public Vector4 colour {
			get {
				Vector4 result;
				InternalCalls.SpriteRendererComponent_GetColour(Entity.ID, out result, GetType().Name);
				return result;
			}
			set { InternalCalls.SpriteRendererComponent_SetColour(Entity.ID, ref value, GetType().Name); }
		}

		public Texture2D texture {
			get { return InternalCalls.SpriteRendererComponent_GetTexture(Entity.ID, GetType().Name); }
			set { InternalCalls.SpriteRendererComponent_SetTexture(Entity.ID, value.native, GetType().Name); }
		}

		public float tilingFactor {
			get {
				float result;
				InternalCalls.SpriteRendererComponent_GetTilingFactor(Entity.ID, out result, GetType().Name);
				return result;
			}
			set { InternalCalls.SpriteRendererComponent_SetTilingFactor(Entity.ID, ref value, GetType().Name); }
		}
	}

	public class CircleRendererComponent : SpriteRendererComponent
	{
		public float thickness {
			get {
				float result;
				InternalCalls.CircleRendererComponent_GetFloatValue(Entity.ID, "thickness", out result);
				return result;
			}
			set { InternalCalls.CircleRendererComponent_SetFloatValue(Entity.ID, "thickness", ref value); }
		}

		public float fade {
			get {
				float result;
				InternalCalls.CircleRendererComponent_GetFloatValue(Entity.ID, "fade", out result);
				return result;
			}
			set { InternalCalls.CircleRendererComponent_SetFloatValue(Entity.ID, "fade", ref value); }
		}
	}

	public class LineRendererComponent : Component
	{
		public Vector3 target {
			get {
				Vector3 result;
				InternalCalls.LineRendererComponent_GetTarget(Entity.ID, out result);
				return result;
			}
			set { InternalCalls.LineRendererComponent_SetTarget(Entity.ID, ref value); }
		}

		public Vector4 colour {
			get {
				Vector4 result;
				InternalCalls.LineRendererComponent_GetColour(Entity.ID, out result);
				return result;
			}
			set { InternalCalls.LineRendererComponent_SetColour(Entity.ID, ref value); }
		}
	}

	public class RigidBody2DComponent : Component
	{
		public enum BodyType
		{
			Static = 0,
			Dynamic = 2,
			Kinematic = 1
		}

		public Vector2 linearVelocity {
			get {
				InternalCalls.Rigidbody2DComponent_GetLinearVelocity(Entity.ID, out Vector2 velocity);
				return velocity;
			}
		}

		public BodyType type {
			get => InternalCalls.Rigidbody2DComponent_GetType(Entity.ID);
			set => InternalCalls.Rigidbody2DComponent_SetType(Entity.ID, value);
		}

		public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake) {
			InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPosition, wake);
		}

		public void ApplyLinearImpulse(Vector2 impulse, bool wake) {
			InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
		}

	}

	public class BoxCollider2DComponent : Component
	{
		public Vector2 offset {
			get {
				Vector2 result;
				InternalCalls.BoxCollider2DComponent_GetVec2(Entity.ID, FieldToQuery.Offset, out result,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
				return result;
			}
			set {
				InternalCalls.BoxCollider2DComponent_SetVec2(Entity.ID, FieldToQuery.Offset, ref value,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
			}
		}

		public Vector2 size {
			get {
				Vector2 result;
				InternalCalls.BoxCollider2DComponent_GetVec2(Entity.ID, FieldToQuery.Size, out result,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
				return result;
			}
			set {
				InternalCalls.BoxCollider2DComponent_SetVec2(Entity.ID, FieldToQuery.Size, ref value,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
			}
		}

		public float density {
			get {
				float result;
				InternalCalls.BoxCollider2DComponent_GetFloat(Entity.ID, FieldToQuery.Density, out result,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
				return result;
			}
			set {
				InternalCalls.BoxCollider2DComponent_SetFloat(Entity.ID, FieldToQuery.Density, ref value,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
			}
		}

		public float friction {
			get {
				float result;
				InternalCalls.BoxCollider2DComponent_GetFloat(Entity.ID, FieldToQuery.Friction, out result,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
				return result;
			}
			set {
				InternalCalls.BoxCollider2DComponent_SetFloat(Entity.ID, FieldToQuery.Friction, ref value,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
			}
		}

		public float restitution {
			get {
				float result;
				InternalCalls.BoxCollider2DComponent_GetFloat(Entity.ID, FieldToQuery.Restitution, out result,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
				return result;
			}
			set {
				InternalCalls.BoxCollider2DComponent_SetFloat(Entity.ID, FieldToQuery.Restitution, ref value,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
			}
		}

		public float restitutionThreshold {
			get {
				float result;
				InternalCalls.BoxCollider2DComponent_GetFloat(Entity.ID, FieldToQuery.RestitutionThreshold, out result,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
				return result;
			}
			set {
				InternalCalls.BoxCollider2DComponent_SetFloat(Entity.ID, FieldToQuery.RestitutionThreshold, ref value,
					GetType() == typeof(BoxCollider2DComponent)
						? FieldToQuery.BoxCollider
						: FieldToQuery.CircleCollider);
			}
		}

		internal enum FieldToQuery
		{
			Density = 0,
			Friction,
			Restitution,
			RestitutionThreshold,
			Offset,
			Size,
			CircleCollider,
			BoxCollider
		}
	}

	public class CircleCollider2DComponent : BoxCollider2DComponent
	{ }

	public class CameraComponent : Component
	{

		public bool primary {
			get {
				bool result;
				InternalCalls.CameraComponent_GetPrimary(Entity.ID, out result);
				return result;
			}

			set { InternalCalls.CameraComponent_SetPrimary(Entity.ID, ref value); }
		}

		public Type type {
			get {
				Type result;
				InternalCalls.CameraComponent_GetType(Entity.ID, out result);
				return result;
			}
			set { InternalCalls.CameraComponent_SetType(Entity.ID, ref value); }
		}

		public void SetViewport(uint width, uint height) {
			InternalCalls.CameraComponent_SetViewport(Entity.ID, width, height);
		}

		public enum Type
		{
			Orthographic = 0,
			Orbit = 1
		}
	}

	public class ScriptComponent : Component
	{
		private string cachedName = null;

		public string ClassName {
			get {
				if (cachedName is null)
					cachedName = InternalCalls.ScriptComponent_GetClassName(Entity.ID);
				return cachedName;
			}
		}
	}

	public class NativeScriptComponent : Component
	{ }

	public class TextComponent : Component
	{
		public string text {
			get {
				InternalCalls.TextComponent_GetText(Entity.ID, out string text);
				return text;
			}
			set { InternalCalls.TextComponent_SetText(Entity.ID, ref value); }
		}
		
		public object font { get; set; }

		public Vector4 colour {
			get {
				InternalCalls.TextComponent_GetColour(Entity.ID, out Vector4 colour);
				return colour;
			}
			set { InternalCalls.TextComponent_SetColour(Entity.ID, ref value); }
		}

		public float lineSpacing {
			get {
				InternalCalls.TextComponent_GetLineSpacing(Entity.ID, out float lineSpacing);
				return lineSpacing;
			}
			set { InternalCalls.TextComponent_SetLineSpacing(Entity.ID, ref value); }
		}

		public float kerning {
			get {
				InternalCalls.TextComponent_GetKerning(Entity.ID, out float kerning);
				return kerning;
			}
			set { InternalCalls.TextComponent_SetKerning(Entity.ID, ref value); }
		}
	}
}
