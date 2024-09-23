using Coral.Managed.Interop;
using System;

namespace Shado
{
    internal static unsafe class InternalCalls
    {
#pragma warning disable CS0649 // Variable is never assigned to

        #region Application

        internal static delegate* unmanaged<void> Application_Quit;
        internal static delegate* unmanaged<float> Application_GetTime;
        internal static delegate* unmanaged<uint> Application_GetWidth;
        internal static delegate* unmanaged<uint> Application_GetHeight;
        internal static delegate* unmanaged<NativeString, Vector2*, void> Application_GetImGuiWindowSize;
        internal static delegate* unmanaged<NativeString, bool> Application_IsImGuiWindowHovered;

        #endregion

        #region Window

        internal static delegate* unmanaged<Vector2*, void> Window_GetPosition;
        internal static delegate* unmanaged<Vector2*, void> Window_GetSize;
        internal static delegate* unmanaged<Vector2*, void> Window_SetSize;
        internal static delegate* unmanaged<int> Window_GetMode;
        internal static delegate* unmanaged<int, void> Window_SetMode;
        internal static delegate* unmanaged<NativeString> Window_GetTitle;
        internal static delegate* unmanaged<NativeString, void> Window_SetTitle;
        internal static delegate* unmanaged<bool> Window_GetVSync;
        internal static delegate* unmanaged<bool, void> Window_SetVSync;
        internal static delegate* unmanaged<float, void> Window_SetOpacity;

        #endregion

        #region Entity

        internal static delegate* unmanaged<ulong, ulong> Entity_GetParent;
        internal static delegate* unmanaged<ulong, ulong, void> Entity_SetParent;
        internal static delegate* unmanaged<ulong, NativeArray<ulong>> Entity_GetChildren;
        internal static delegate* unmanaged<ulong, ReflectionType, void> Entity_CreateComponent;
        internal static delegate* unmanaged<ulong, ReflectionType, bool> Entity_HasComponent;
        internal static delegate* unmanaged<ulong, ReflectionType, bool> Entity_RemoveComponent;
        internal static delegate* unmanaged<NativeString, ulong> Entity_FindEntityByName;

        #endregion

        #region Prefab

        internal static delegate* unmanaged<ulong, Vector3, ulong> Prefab_Instantiate;

        #endregion

        #region Scene

        internal static delegate* unmanaged<ulong, bool> Scene_IsEntityValid;
        internal static delegate* unmanaged<NativeString, NativeString> Scene_LoadScene;
        internal static delegate* unmanaged<NativeArray<ulong>> Scene_GetAllEntities;

        #endregion

        #region TagComponent

        internal static delegate* unmanaged<ulong, NativeString> TagComponent_GetTag;
        internal static delegate* unmanaged<ulong, NativeString, void> TagComponent_SetTag;

        #endregion

        #region TransformComponent

        internal static delegate* unmanaged<ulong, Transform*, void> TransformComponent_GetTransform;
        internal static delegate* unmanaged<ulong, Transform*, void> TransformComponent_SetTransform;

        #endregion

        #region ScriptComponent

        internal static delegate* unmanaged<ulong, NativeInstance<object>> ScriptComponent_GetInstance;

        #endregion

        #region CameraComponent

        internal static delegate* unmanaged<ulong, bool> CameraComponent_GetPrimary;
        internal static delegate* unmanaged<ulong, bool, void> CameraComponent_SetPrimary;
        internal static delegate* unmanaged<ulong, CameraComponent.Type> CameraComponent_GetType;
        internal static delegate* unmanaged<ulong, CameraComponent.Type, void> CameraComponent_SetType;
        internal static delegate* unmanaged<ulong, uint, uint, void> CameraComponent_SetViewportSize;

        #endregion

        #region SpriteRendererComponent

        internal static delegate* unmanaged<ulong, Vector4*, void> SpriteRendererComponent_GetColor;
        internal static delegate* unmanaged<ulong, Vector4*, void> SpriteRendererComponent_SetColor;
        internal static delegate* unmanaged<ulong, float> SpriteRendererComponent_GetTilingFactor;
        internal static delegate* unmanaged<ulong, float, void> SpriteRendererComponent_SetTilingFactor;
        internal static delegate* unmanaged<ulong, ulong> SpriteRendererComponent_GetTexture;
        internal static delegate* unmanaged<ulong, ulong, void> SpriteRendererComponent_SetTexture;
        internal static delegate* unmanaged<ulong, ulong> SpriteRendererComponent_GetShader;
        internal static delegate* unmanaged<ulong, ulong, void> SpriteRendererComponent_SetShader;

        #endregion

        #region CircleRendererComponent

        internal static delegate* unmanaged<ulong, Vector4*, void> CircleRendererComponent_GetColor;
        internal static delegate* unmanaged<ulong, Vector4*, void> CircleRendererComponent_SetColor;
        internal static delegate* unmanaged<ulong, float> CircleRendererComponent_GetThickness;
        internal static delegate* unmanaged<ulong, float, void> CircleRendererComponent_SetThickness;
        internal static delegate* unmanaged<ulong, float> CircleRendererComponent_GetFade;
        internal static delegate* unmanaged<ulong, float, void> CircleRendererComponent_SetFade;

        #endregion

        #region LineRendererComponent

        internal static delegate* unmanaged<ulong, Vector3*, void> LineRendererComponent_GetTarget;
        internal static delegate* unmanaged<ulong, Vector3*, void> LineRendererComponent_SetTarget;
        internal static delegate* unmanaged<ulong, Vector4*, void> LineRendererComponent_GetColour;
        internal static delegate* unmanaged<ulong, Vector4*, void> LineRendererComponent_SetColour;

        #endregion

        #region RigidBody2DComponent

        internal static delegate* unmanaged<ulong, Vector2*, void> RigidBody2DComponent_GetLinearVelocity;
        internal static delegate* unmanaged<ulong, RigidBody2DComponent.BodyType> RigidBody2DComponent_GetBodyType;

        internal static delegate* unmanaged<ulong, RigidBody2DComponent.BodyType, void>
            RigidBody2DComponent_SetBodyType;

        internal static delegate* unmanaged<ulong, Vector2, Vector2, bool, void>
            Rigidbody2DComponent_ApplyLinearImpulse;

        internal static delegate* unmanaged<ulong, Vector2, bool, void> Rigidbody2DComponent_ApplyLinearImpulseToCenter;

        #endregion

        #region BoxCollider2DComponent and CircleCollider2DComponent

        internal static delegate* unmanaged<ulong, Vector2*, void> BoxCollider2DComponent_GetOffset;
        internal static delegate* unmanaged<ulong, Vector2*, void> BoxCollider2DComponent_SetOffset;
        internal static delegate* unmanaged<ulong, Vector2*, void> BoxCollider2DComponent_GetSize;
        internal static delegate* unmanaged<ulong, Vector2*, void> BoxCollider2DComponent_SetSize;
        internal static delegate* unmanaged<ulong, float> BoxCollider2DComponent_GetDensity;
        internal static delegate* unmanaged<ulong, float, void> BoxCollider2DComponent_SetDensity;
        internal static delegate* unmanaged<ulong, float> BoxCollider2DComponent_GetFriction;
        internal static delegate* unmanaged<ulong, float, void> BoxCollider2DComponent_SetFriction;
        internal static delegate* unmanaged<ulong, float> BoxCollider2DComponent_GetRestitution;
        internal static delegate* unmanaged<ulong, float, void> BoxCollider2DComponent_SetRestitution;
        internal static delegate* unmanaged<ulong, float> BoxCollider2DComponent_GetRestitutionThreshold;
        internal static delegate* unmanaged<ulong, float, void> BoxCollider2DComponent_SetRestitutionThreshold;

        #endregion

        #region TextComponent

        internal static delegate* unmanaged<ulong, NativeString> TextComponent_GetText;
        internal static delegate* unmanaged<ulong, NativeString, void> TextComponent_SetText;
        internal static delegate* unmanaged<ulong, Vector4*, void> TextComponent_GetColor;
        internal static delegate* unmanaged<ulong, Vector4*, void> TextComponent_SetColor;
        internal static delegate* unmanaged<ulong, float> TextComponent_GetLineSpacing;
        internal static delegate* unmanaged<ulong, float, void> TextComponent_SetLineSpacing;
        internal static delegate* unmanaged<ulong, float> TextComponent_GetKerning;
        internal static delegate* unmanaged<ulong, float, void> TextComponent_SetKerning;

        #endregion

        #region Texture2D

        internal static delegate* unmanaged<NativeString, ulong> Texture2D_Create;

        #endregion

        #region Shader

        internal static delegate* unmanaged<NativeString, ulong> Shader_Create;
        internal static delegate* unmanaged<ulong, NativeString, int, void> Shader_SetInt;
        internal static delegate* unmanaged<ulong, NativeString, float, void> Shader_SetFloat;
        internal static delegate* unmanaged<ulong, NativeString, Vector2, void> Shader_SetFloat2;
        internal static delegate* unmanaged<ulong, NativeString, Vector3, void> Shader_SetFloat3;
        internal static delegate* unmanaged<ulong, NativeString, Vector4, void> Shader_SetFloat4;

        #endregion

        #region Log

        internal static delegate* unmanaged<Log.LogLevel, NativeString, void> Log_LogMessage;

        #endregion

        #region Input

        internal static delegate* unmanaged<KeyCode, Bool32> Input_IsKeyPressed;
        internal static delegate* unmanaged<int, bool> Input_IsMouseButtonPressed;
        internal static delegate* unmanaged<Vector2*, void> Input_GetMousePosition;

        #endregion

#pragma warning restore CS0649
    }
}