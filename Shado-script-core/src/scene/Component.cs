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
                    return InternalCalls.TagComponent_GetTag(Entity.ID);
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
                IntPtr texturePtr;
                unsafe {
                    texturePtr = (IntPtr)InternalCalls.SpriteRendererComponent_GetTexture(Entity.ID);
                }

                return new Texture2D(texturePtr);
            }
            set {
                unsafe {
                    InternalCalls.SpriteRendererComponent_SetTexture(Entity.ID, (int*)value.native);
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

    public class CircleRendererComponent : SpriteRendererComponent
    {
        // public float thickness {
        //     get {
        //         float result;
        //         InternalCalls.CircleRendererComponent_GetFloatValue(Entity.ID, "thickness", out result);
        //         return result;
        //     }
        //     set { InternalCalls.CircleRendererComponent_SetFloatValue(Entity.ID, "thickness", ref value); }
        // }
        //
        // public float fade {
        //     get {
        //         float result;
        //         InternalCalls.CircleRendererComponent_GetFloatValue(Entity.ID, "fade", out result);
        //         return result;
        //     }
        //     set { InternalCalls.CircleRendererComponent_SetFloatValue(Entity.ID, "fade", ref value); }
        // }
    }

    public class LineRendererComponent : Component
    {
        // public Vector3 target {
        //     get {
        //         Vector3 result;
        //         InternalCalls.LineRendererComponent_GetTarget(Entity.ID, out result);
        //         return result;
        //     }
        //     set { InternalCalls.LineRendererComponent_SetTarget(Entity.ID, ref value); }
        // }
        //
        // public Vector4 colour {
        //     get {
        //         Vector4 result;
        //         InternalCalls.LineRendererComponent_GetColour(Entity.ID, out result);
        //         return result;
        //     }
        //     set { InternalCalls.LineRendererComponent_SetColour(Entity.ID, ref value); }
        // }
    }

    public class RigidBody2DComponent : Component
    {
        public enum BodyType
        {
            Static = 0,
            Dynamic = 2,
            Kinematic = 1
        }

        // public Vector2 linearVelocity {
        //     get {
        //         InternalCalls.Rigidbody2DComponent_GetLinearVelocity(Entity.ID, out Vector2 velocity);
        //         return velocity;
        //     }
        // }

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

        // public void ApplyLinearImpulse(Vector2 impulse, Vector2 worldPosition, bool wake) {
        //     InternalCalls.Rigidbody2DComponent_ApplyLinearImpulse(Entity.ID, ref impulse, ref worldPosition, wake);
        // }
        //
        // public void ApplyLinearImpulse(Vector2 impulse, bool wake) {
        //     InternalCalls.Rigidbody2DComponent_ApplyLinearImpulseToCenter(Entity.ID, ref impulse, wake);
        // }
    }

    public class BoxCollider2DComponent : Component
    {
        // public Vector2 offset {
        //     get {
        //         Vector2 result;
        //         InternalCalls.BoxCollider2DComponent_GetVec2(Entity.ID, FieldToQuery.Offset, out result,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //         return result;
        //     }
        //     set {
        //         InternalCalls.BoxCollider2DComponent_SetVec2(Entity.ID, FieldToQuery.Offset, ref value,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //     }
        // }
        //
        // public Vector2 size {
        //     get {
        //         Vector2 result;
        //         InternalCalls.BoxCollider2DComponent_GetVec2(Entity.ID, FieldToQuery.Size, out result,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //         return result;
        //     }
        //     set {
        //         InternalCalls.BoxCollider2DComponent_SetVec2(Entity.ID, FieldToQuery.Size, ref value,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //     }
        // }
        //
        // public float density {
        //     get {
        //         float result;
        //         InternalCalls.BoxCollider2DComponent_GetFloat(Entity.ID, FieldToQuery.Density, out result,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //         return result;
        //     }
        //     set {
        //         InternalCalls.BoxCollider2DComponent_SetFloat(Entity.ID, FieldToQuery.Density, ref value,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //     }
        // }
        //
        // public float friction {
        //     get {
        //         float result;
        //         InternalCalls.BoxCollider2DComponent_GetFloat(Entity.ID, FieldToQuery.Friction, out result,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //         return result;
        //     }
        //     set {
        //         InternalCalls.BoxCollider2DComponent_SetFloat(Entity.ID, FieldToQuery.Friction, ref value,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //     }
        // }
        //
        // public float restitution {
        //     get {
        //         float result;
        //         InternalCalls.BoxCollider2DComponent_GetFloat(Entity.ID, FieldToQuery.Restitution, out result,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //         return result;
        //     }
        //     set {
        //         InternalCalls.BoxCollider2DComponent_SetFloat(Entity.ID, FieldToQuery.Restitution, ref value,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //     }
        // }
        //
        // public float restitutionThreshold {
        //     get {
        //         float result;
        //         InternalCalls.BoxCollider2DComponent_GetFloat(Entity.ID, FieldToQuery.RestitutionThreshold, out result,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //         return result;
        //     }
        //     set {
        //         InternalCalls.BoxCollider2DComponent_SetFloat(Entity.ID, FieldToQuery.RestitutionThreshold, ref value,
        //             GetType() == typeof(BoxCollider2DComponent)
        //                 ? FieldToQuery.BoxCollider
        //                 : FieldToQuery.CircleCollider);
        //     }
        // }
    }

    public class CircleCollider2DComponent : BoxCollider2DComponent
    { }

    public class CameraComponent : Component
    {
        // public bool primary {
        //     get {
        //         bool result;
        //         InternalCalls.CameraComponent_GetPrimary(Entity.ID, out result);
        //         return result;
        //     }
        //
        //     set { InternalCalls.CameraComponent_SetPrimary(Entity.ID, ref value); }
        // }
        //
        // public Type type {
        //     get {
        //         Type result;
        //         InternalCalls.CameraComponent_GetType(Entity.ID, out result);
        //         return result;
        //     }
        //     set { InternalCalls.CameraComponent_SetType(Entity.ID, ref value); }
        // }
        //
        // public void SetViewport(uint width, uint height) {
        //     InternalCalls.CameraComponent_SetViewport(Entity.ID, width, height);
        // }
        //
        // public enum Type
        // {
        //     Orthographic = 0,
        //     Orbit = 1
        // }
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
                    return InternalCalls.TextComponent_GetText(Entity.ID);
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

        // public float lineSpacing {
        //     get {
        //         InternalCalls.TextComponent_GetLineSpacing(Entity.ID, out float lineSpacing);
        //         return lineSpacing;
        //     }
        //     set { InternalCalls.TextComponent_SetLineSpacing(Entity.ID, ref value); }
        // }
        //
        // public float kerning {
        //     get {
        //         InternalCalls.TextComponent_GetKerning(Entity.ID, out float kerning);
        //         return kerning;
        //     }
        //     set { InternalCalls.TextComponent_SetKerning(Entity.ID, ref value); }
        // }
    }
}