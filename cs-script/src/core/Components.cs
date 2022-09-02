using System;
using Shado.math;
using System.Runtime.CompilerServices;

namespace Shado
{
    public abstract class Component
    {
        public Entity Entity { get; set; }
    }

    public class TagComponent : Component { 
        public string Tag {
            get {
                return GetTag_Native(Entity.Id);
            } 
            set { 
                SetTag_Native(Entity.Id, value);
            } 
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern string GetTag_Native(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetTag_Native(ulong entityID, string tag);
    }

    public class TransformComponent : Component
    {
        public Vector3 Position
        {
            get
            {
                Vector3 result;
                GetPosition_Native(Entity.Id, out result);
                return result;
            }
            set
            {
                SetPosition_Native(Entity.Id, ref value);
            }
        }

        public Vector3 Rotation
        {
            get
            {
                Vector3 result;
                GetRotation_Native(Entity.Id, out result);
                return result;
            }
            set
            {
                SetRotation_Native(Entity.Id, ref value);
            }
        }

        public Vector3 Scale
        {
            get
            {
                Vector3 result;
                GetScale_Native(Entity.Id, out result);
                return result;
            }
            set
            {
                SetScale_Native(Entity.Id, ref value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetPosition_Native(ulong entityID, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetPosition_Native(ulong entityID, ref Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetRotation_Native(ulong entityID, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetRotation_Native(ulong entityID, ref Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetScale_Native(ulong entityID, out Vector3 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetScale_Native(ulong entityID, ref Vector3 result);
    }

    public class SpriteRendererComponent : Component
    {
        public Vector4 Color
        {
            get
            {
                Vector4 result;
                GetColor_Native(Entity.Id, out result, GetType());
                return result;
            }
            set
            {
                SetColor_Native(Entity.Id, ref value, GetType());
            }
        }

        public Texture2D Texture {
            get
            {
                return GetTexture_Native(Entity, GetType());
            }
            set
            {
                SetTexture_Native(Entity.Id, value.Path, GetType());
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void GetColor_Native(ulong entityID, out Vector4 result, Type klass);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void SetColor_Native(ulong entityID, ref Vector4 result, Type klass);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void SetTexture_Native(ulong entityID, string texturePath, Type klass);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern Texture2D GetTexture_Native(EntityDesc entity, Type klass);
    }

    public class CircleRendererComponent : SpriteRendererComponent
    {
        public float Thickness
        {
            get
            {
                float result;
                GetFloatVal_Native(Entity.Id, "thickness", out result);
                return result;
            }
            set
            {
                SetFloatVal_Native(Entity.Id, "thickness", ref value);
            }
        }

        public float Fade
        {
            get
            {
                float result;
                GetFloatVal_Native(Entity.Id, "fade", out result);
                return result;
            }
            set
            {
                SetFloatVal_Native(Entity.Id, "fade", ref value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void GetFloatVal_Native(ulong entityID, string varName, out float result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        private static extern void SetFloatVal_Native(ulong entityID, string varName, ref float result);
    }

    public class RigidBody2DComponent : Component {
        public enum BodyType { 
            STATIC = 0,
            KINEMATIC, DYNAMIC
        }

        public BodyType Type {
            get
            {
                int result;
                GetBodyType_Native(Entity.Id, out result);
                return (BodyType)result;
            }
            set
            {
                SetBodyType_Native(Entity.Id, ref value);
            }
        }

        public bool FixedRotation
        {
            get
            {
                bool result;
                GetFixedRotation_Native(Entity.Id, out result);
                return result;
            }
            set
            {
                SetFixedRotation_Native(Entity.Id, ref value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetBodyType_Native(ulong entityID, out int result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetBodyType_Native(ulong entityID, ref BodyType result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void GetFixedRotation_Native(ulong entityID, out bool result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        public static extern void SetFixedRotation_Native(ulong entityID, ref bool result);
    }

    public class BoxCollider2DComponent : Component {

        public Vector2 Offset
        {
            get
            {
                Vector2 result;
                GetOffset_Native(Entity.Id, out result);
                return result;
            }
            set
            {
                SetOffset_Native(Entity.Id, ref value);
            }
        }

        public Vector2 Size
        {
            get
            {
                Vector2 result;
                GetSize_Native(Entity.Id, out result);
                return result;
            }
            set
            {
                SetSize_Native(Entity.Id, ref value);
            }
        }

        public float Density
        {
            get
            {
                float result;
                GetFloatVal_Native(Entity.Id, "density", out result);
                return result;
            }
            set
            {
                SetFloatVal_Native(Entity.Id, "density", ref value);
            }
        }

        public float Friction
        {
            get
            {
                float result;
                GetFloatVal_Native(Entity.Id, "friction", out result);
                return result;
            }
            set
            {
                SetFloatVal_Native(Entity.Id, "friction", ref value);
            }
        }

        public float Restitution
        {
            get
            {
                float result;
                GetFloatVal_Native(Entity.Id, "restitution", out result);
                return result;
            }
            set
            {
                SetFloatVal_Native(Entity.Id, "restitution", ref value);
            }
        }

        public float RestitutionThreshold
        {
            get
            {
                float result;
                GetFloatVal_Native(Entity.Id, "restitutionThreshold", out result);
                return result;
            }
            set
            {
                SetFloatVal_Native(Entity.Id, "restitutionThreshold", ref value);
            }
        }

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void GetOffset_Native(ulong entityID, out Vector2 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void SetOffset_Native(ulong entityID, ref Vector2 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void GetSize_Native(ulong entityID, out Vector2 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void SetSize_Native(ulong entityID, ref Vector2 result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void GetFloatVal_Native(ulong entityID, string varName, out float result);

        [MethodImpl(MethodImplOptions.InternalCall)]
        protected static extern void SetFloatVal_Native(ulong entityID, string varName, ref float result);

    }

    public class CircleCollider2DComponent : BoxCollider2DComponent
    {
        public new float Size
        {
            get
            {
                Vector2 result;
                GetSize_Native(Entity.Id, out result);
                return result.x;
            }
            set
            {
                Vector2 temp = new Vector2(value);
                SetSize_Native(Entity.Id, ref temp);
            }
        }
    }
}
