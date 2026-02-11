using System;
using System.Collections.Generic;
using System.Linq;
using System.Reflection;
using Coral.Managed.Interop;

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
                unsafe {
                    return InternalCalls.TagComponent_GetTag(Entity.ID)!;
                }
            }
            set {
                unsafe {
                    InternalCalls.TagComponent_SetTag(Entity.ID, value);
                }
            }
        }
    }

    public class TransformComponent : Component
    {
        public Vector3 position {
            get {
                Vector3 result;
                unsafe {
                    Transform transform;
                    InternalCalls.TransformComponent_GetTransform(Entity.ID, &transform);
                    result = transform.Position;
                }

                return result;
            }

            set {
                unsafe {
                    Transform transform = new Transform {
                        Position = value,
                        Rotation = rotation,
                        Scale = scale
                    };
                    InternalCalls.TransformComponent_SetTransform(Entity.ID, &transform);
                }
            }
        }

        public Vector3 rotation {
            get {
                Vector3 result;
                unsafe {
                    Transform transform;
                    InternalCalls.TransformComponent_GetTransform(Entity.ID, &transform);
                    result = transform.Rotation;
                }

                return result;
            }
            set {
                Transform transform = new Transform {
                    Position = position,
                    Rotation = value,
                    Scale = scale
                };
                unsafe {
                    InternalCalls.TransformComponent_SetTransform(Entity.ID, &transform);
                }
            }
        }

        public Vector3 scale {
            get {
                Vector3 result;
                unsafe {
                    Transform transform;
                    InternalCalls.TransformComponent_GetTransform(Entity.ID, &transform);
                    result = transform.Scale;
                }

                return result;
            }
            set {
                Transform transform = new Transform {
                    Position = position,
                    Rotation = rotation,
                    Scale = value
                };
                unsafe {
                    InternalCalls.TransformComponent_SetTransform(Entity.ID, &transform);
                }
            }
        }

        public Entity? parent {
            get {
                unsafe {
                    ulong parentId = InternalCalls.Entity_GetParent(Entity.ID);
                    return InternalCalls.Scene_IsEntityValid(parentId) ? new Entity(parentId) : null;
                }
            }
            set {
                unsafe {
                    InternalCalls.Entity_SetParent(Entity.ID, value?.ID ?? 0);
                }
            }
        }
    }

    public class SpriteRendererComponent : Component
    {
        public Vector4 colour {
            get {
                Vector4 result;
                unsafe {
                    InternalCalls.SpriteRendererComponent_GetColor(Entity.ID, &result);
                }

                return result;
            }
            set {
                unsafe {
                    InternalCalls.SpriteRendererComponent_SetColor(Entity.ID, &value);
                }
            }
        }

        public Texture2D texture {
            get {
                unsafe {
                    return new Texture2D(InternalCalls.SpriteRendererComponent_GetTexture(Entity.ID));
                }
            }
            set {
                unsafe {
                    InternalCalls.SpriteRendererComponent_SetTexture(Entity.ID, value.handle);
                }
            }
        }

        public Shader shader {
            get {
                unsafe {
                    return new Shader(InternalCalls.SpriteRendererComponent_GetShader(Entity.ID));
                }
            }
            set {
                unsafe {
                    InternalCalls.SpriteRendererComponent_SetShader(Entity.ID, value.handle);
                }
            }
        }

        public float tilingFactor {
            get {
                unsafe {
                    return InternalCalls.SpriteRendererComponent_GetTilingFactor(Entity.ID);
                }
            }
            set {
                unsafe {
                    InternalCalls.SpriteRendererComponent_SetTilingFactor(Entity.ID, value);
                }
            }
        }
    }

    public class CircleRendererComponent : Component
    {
        public Vector4 colour {
            get {
                Vector4 result;
                unsafe {
                    InternalCalls.CircleRendererComponent_GetColor(Entity.ID, &result);
                }

                return result;
            }
            set {
                unsafe {
                    InternalCalls.CircleRendererComponent_SetColor(Entity.ID, &value);
                }
            }
        }

        public float thickness {
            get {
                unsafe {
                    return InternalCalls.CircleRendererComponent_GetThickness(Entity.ID);
                }
            }
            set {
                unsafe {
                    InternalCalls.CircleRendererComponent_SetThickness(Entity.ID, value);
                }
            }
        }

        public float fade {
            get {
                unsafe {
                    return InternalCalls.CircleRendererComponent_GetFade(Entity.ID);
                }
            }
            set {
                unsafe {
                    InternalCalls.CircleRendererComponent_SetFade(Entity.ID, value);
                }
            }
        }
    }

    public class LineRendererComponent : Component
    {
        public Vector3 target {
            get {
                unsafe {
                    Vector3 result;
                    InternalCalls.LineRendererComponent_GetTarget(Entity.ID, &result);
                    return result;
                }
            }
            set {
                unsafe {
                    InternalCalls.LineRendererComponent_SetTarget(Entity.ID, &value);
                }
            }
        }

        public Vector4 colour {
            get {
                unsafe {
                    Vector4 result;
                    InternalCalls.LineRendererComponent_GetColour(Entity.ID, &result);
                    return result;
                }
            }
            set {
                unsafe {
                    InternalCalls.LineRendererComponent_SetColour(Entity.ID, &value);
                }
            }
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
                Vector2 velocity;
                unsafe {
                    InternalCalls.RigidBody2DComponent_GetLinearVelocity(Entity.ID, &velocity);
                }

                return velocity;
            }
        }

        public BodyType type {
            get {
                unsafe {
                    return InternalCalls.RigidBody2DComponent_GetBodyType(Entity.ID);
                }
            }
            set {
                unsafe {
                    InternalCalls.RigidBody2DComponent_SetBodyType(Entity.ID, value);
                }
            }
        }

        public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake) {
            unsafe {
                InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, impulse, worldPosition, wake);
            }
        }

        public void ApplyLinearImpulse(Vector2 impulse, bool wake) {
            unsafe {
                InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, impulse, wake);
            }
        }
    }

    public class BoxCollider2DComponent : Component
    {
        public Vector2 offset {
            get {
                unsafe {
                    Vector2 result;
                    InternalCalls.BoxCollider2DComponent_GetOffset(Entity.ID, &result);
                    return result;
                }
            }
            set {
                unsafe {
                    InternalCalls.BoxCollider2DComponent_SetOffset(Entity.ID, &value);
                }
            }
        }

        public Vector2 size {
            get {
                unsafe {
                    Vector2 result;
                    InternalCalls.BoxCollider2DComponent_GetSize(Entity.ID, &result);
                    return result;
                }
            }
            set {
                unsafe {
                    InternalCalls.BoxCollider2DComponent_SetSize(Entity.ID, &value);
                }
            }
        }

        public float density {
            get {
                unsafe {
                    return InternalCalls.BoxCollider2DComponent_GetDensity(Entity.ID);
                }
            }
            set {
                unsafe {
                    InternalCalls.BoxCollider2DComponent_SetDensity(Entity.ID, value);
                }
            }
        }

        public float friction {
            get {
                unsafe {
                    return InternalCalls.BoxCollider2DComponent_GetFriction(Entity.ID);
                }
            }

            set {
                unsafe {
                    InternalCalls.BoxCollider2DComponent_SetFriction(Entity.ID, value);
                }
            }
        }

        public float restitution {
            get {
                unsafe {
                    return InternalCalls.BoxCollider2DComponent_GetRestitution(Entity.ID);
                }
            }
            set {
                unsafe {
                    InternalCalls.BoxCollider2DComponent_SetRestitution(Entity.ID, value);
                }
            }
        }

        public float restitutionThreshold {
            get {
                unsafe {
                    return InternalCalls.BoxCollider2DComponent_GetRestitutionThreshold(Entity.ID);
                }
            }
            set {
                unsafe {
                    InternalCalls.BoxCollider2DComponent_SetRestitutionThreshold(Entity.ID, value);
                }
            }
        }
    }

    public class CircleCollider2DComponent : BoxCollider2DComponent
    { }

    public class CameraComponent : Component
    {
        public bool primary {
            get {
                unsafe {
                    return InternalCalls.CameraComponent_GetPrimary(Entity.ID);
                }
            }

            set {
                unsafe {
                    InternalCalls.CameraComponent_SetPrimary(Entity.ID, value);
                }
            }
        }

        public Type type {
            get {
                unsafe {
                    return InternalCalls.CameraComponent_GetType(Entity.ID);
                }
            }
            set {
                unsafe {
                    InternalCalls.CameraComponent_SetType(Entity.ID, value);
                }
            }
        }

        public Matrix4x4 view
        {
            get
            {
                unsafe
                {
                    return InternalCalls.CameraComponent_GetView(Entity.ID);
                }
            }
        }
        
        public Matrix4x4 projection
        {
            get
            {
                unsafe
                {
                    return InternalCalls.CameraComponent_GetProjection(Entity.ID);
                }
            }
        }
        
        public void SetViewport(uint width, uint height) {
            unsafe {
                InternalCalls.CameraComponent_SetViewportSize(Entity.ID, width, height);
            }
        }

        public enum Type
        {
            Orthographic = 0,
            Orbit = 1
        }
    }

    public class ScriptComponent : Component
    {
        public NativeInstance<object> Instance {
            get {
                unsafe {
                    return InternalCalls.ScriptComponent_GetInstance(Entity.ID);
                }
            }
        }
    }

    public class NativeScriptComponent : Component
    { }

    public class TextComponent : Component
    {
        public string text {
            get {
                unsafe {
                    return InternalCalls.TextComponent_GetText(Entity.ID)!;
                }
            }
            set {
                unsafe {
                    InternalCalls.TextComponent_SetText(Entity.ID, value);
                }
            }
        }

        //public object font { get; set; }

        public Vector4 colour {
            get {
                Vector4 result;
                unsafe {
                    InternalCalls.TextComponent_GetColor(Entity.ID, &result);
                }

                return result;
            }
            set {
                unsafe {
                    InternalCalls.TextComponent_SetColor(Entity.ID, &value);
                }
            }
        }

        public float lineSpacing {
            get {
                unsafe {
                    return InternalCalls.TextComponent_GetLineSpacing(Entity.ID);
                }
            }
            set {
                unsafe {
                    InternalCalls.TextComponent_SetLineSpacing(Entity.ID, value);
                }
            }
        }

        public float kerning {
            get {
                unsafe {
                    return InternalCalls.TextComponent_GetKerning(Entity.ID);
                }
            }
            set {
                unsafe {
                    InternalCalls.TextComponent_SetKerning(Entity.ID, value);
                }
            }
        }
    }
}