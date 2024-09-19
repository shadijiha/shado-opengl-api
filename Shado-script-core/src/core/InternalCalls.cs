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

        #endregion


        #region Entity

        internal static delegate* unmanaged<ulong, ulong> Entity_GetParent;
        internal static delegate* unmanaged<ulong, ulong, void> Entity_SetParent;
        internal static delegate* unmanaged<ulong, NativeArray<ulong>> Entity_GetChildren;
        internal static delegate* unmanaged<ulong, ReflectionType, void> Entity_CreateComponent;
        internal static delegate* unmanaged<ulong, ReflectionType, bool> Entity_HasComponent;
        internal static delegate* unmanaged<ulong, ReflectionType, bool> Entity_RemoveComponent;

        #endregion

        #region Scene

        internal static delegate* unmanaged<ulong, bool> Scene_IsEntityValid;

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

        #endregion

        #region SpriteRendererComponent

        internal static delegate* unmanaged<ulong, Vector4*, void> SpriteRendererComponent_GetColor;
        internal static delegate* unmanaged<ulong, Vector4*, void> SpriteRendererComponent_SetColor;
        internal static delegate* unmanaged<ulong, float> SpriteRendererComponent_GetTilingFactor;
        internal static delegate* unmanaged<ulong, float, void> SpriteRendererComponent_SetTilingFactor;
        internal static delegate* unmanaged<ulong, int*> SpriteRendererComponent_GetTexture;
        internal static delegate* unmanaged<ulong, int*, void> SpriteRendererComponent_SetTexture;

        #endregion

        #region RigidBody2DComponent

        internal static delegate* unmanaged<ulong, RigidBody2DComponent.BodyType> RigidBody2DComponent_GetBodyType;

        internal static delegate* unmanaged<ulong, RigidBody2DComponent.BodyType, void>
            RigidBody2DComponent_SetBodyType;

        #endregion

        #region TextComponent

        internal static delegate* unmanaged<ulong, NativeString> TextComponent_GetText;
        internal static delegate* unmanaged<ulong, NativeString, void> TextComponent_SetText;
        internal static delegate* unmanaged<ulong, Vector4*, void> TextComponent_GetColor;
        internal static delegate* unmanaged<ulong, Vector4*, void> TextComponent_SetColor;

        #endregion

        #region Texture2D

        internal static delegate* unmanaged<NativeString, IntPtr> Texture2D_Create;

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