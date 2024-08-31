using System;
using System.Runtime.CompilerServices;

namespace Shado
{
	public static class InternalCalls
	{
        #region Entity
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static bool Entity_HasComponent(ulong entityID, Type componentType);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_RemoveComponent(ulong entityID, Type componentType);

        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_AddComponent(ulong entityID, Type componentType);

		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_FindEntityByName(string name);

        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_Destroy(ulong entityID);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong Entity_CreateEntityId();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Entity_InvokeScriptEngineCreate(object monoObject, ulong entityId);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static object GetScriptInstance(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Entity_GetChildren(ulong entityID, ref ulong[] childrenIds, Type arrayType);
        
        #endregion

        /**
		 * Tag
		 **/
        #region Tag
        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TagComponent_GetTag(ulong entityID, out string tag);

        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TagComponent_SetTag(ulong entityID, ref string tag);
        #endregion

        /**
		 * Transform
		 **/
		#region Transform
        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetTranslation(ulong entityID, out Vector3 translation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetTranslation(ulong entityID, ref Vector3 translation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetRotation(ulong entityID, out Vector3 rotation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetRotation(ulong entityID, ref Vector3 rotation);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_GetScale(ulong entityID, out Vector3 scale);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void TransformComponent_SetScale(ulong entityID, ref Vector3 scale);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong TransformComponent_GetParentId(ulong entityID);

        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void TransformComponent_SetParentId(ulong entityID, ulong parentId);
        #endregion

        /**
		 * SpriteRenderer
		 */
        #region SpriteRenderer
        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetColour(ulong entityID, out Vector4 result, string klass);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetColour(ulong entityID, ref Vector4 result, string klass);

        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static Texture2D SpriteRendererComponent_GetTexture(ulong entityID, string klass);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetTexture(ulong entityID, IntPtr native, string klass);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_GetTilingFactor(ulong entityID, out float result, string klass);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void SpriteRendererComponent_SetTilingFactor(ulong entityID, ref float result, string klass);
        #endregion

        /**
		 * CircleRenderer
		 */
		#region CircleRenderer
        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_GetFloatValue(ulong entityId, string fieldName, out float val);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CircleRendererComponent_SetFloatValue(ulong entityId, string fieldName, ref float val);
        #endregion

        
        /**
         * LineRenderer
         */
        #region LineRenderer
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void LineRendererComponent_GetTarget(ulong entityId, out Vector3 val);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void LineRendererComponent_SetTarget(ulong entityId, ref Vector3 val);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void LineRendererComponent_GetColour(ulong entityId, out Vector4 val);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void LineRendererComponent_SetColour(ulong entityId, ref Vector4 val);
        #endregion

        /**
		 * Rigidbody2D
		 **/
		#region Rigidbody2D
        [MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulse(ulong entityID, ref Vector2 impulse, ref Vector2 point, bool wake);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_GetLinearVelocity(ulong entityID, out Vector2 linearVelocity);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static RigidBody2DComponent.BodyType Rigidbody2DComponent_GetType(ulong entityID);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_SetType(ulong entityID, RigidBody2DComponent.BodyType type);
		[MethodImplAttribute(MethodImplOptions.InternalCall)]
		internal extern static void Rigidbody2DComponent_ApplyLinearImpulseToCenter(ulong entityID, ref Vector2 impulse, bool wake);
        #endregion

        /**
		 * BoxCollider2D
		 */
        #region BoxCollider2D
        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetVec2(ulong entityId, BoxCollider2DComponent.FieldToQuery field, out Vector2 offset, BoxCollider2DComponent.FieldToQuery componentType);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetVec2(ulong entityId, BoxCollider2DComponent.FieldToQuery field, ref Vector2 offset, BoxCollider2DComponent.FieldToQuery componentType);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_GetFloat(ulong entityId, BoxCollider2DComponent.FieldToQuery field, out float offset, BoxCollider2DComponent.FieldToQuery componentType);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void BoxCollider2DComponent_SetFloat(ulong entityId, BoxCollider2DComponent.FieldToQuery field, ref float offset, BoxCollider2DComponent.FieldToQuery componentType);
        #endregion

        /**
		 *  CameraComponent
		 */
        #region CameraComponent
        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_GetPrimary(ulong entityId, out bool primary);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetPrimary(ulong entityId, ref bool primary);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_GetType(ulong entityId, out CameraComponent.Type type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetType(ulong entityId, ref CameraComponent.Type type);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void CameraComponent_SetViewport(ulong entityID, uint width, uint height);
		#endregion

		/**
		 * Script component
		 */
		#region ScriptComponent

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string ScriptComponent_GetClassName(ulong entityID);
		
		#endregion

        /**
		 * Input
		 **/
        #region Input
        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static bool Input_IsKeyDown(KeyCode keycode);
        
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Input_GetMousePos(ref Vector2 pos);
        #endregion

        /**
		 * Texture2D
		 */
        #region Texture2D
        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Texture2D_Create(string filepath, out IntPtr handle);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Texture2D_Destroy(IntPtr native);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Texture2D_Reset(IntPtr ptr, string filepath, out IntPtr newHandle);
        #endregion
        
		/**
		 * Shader
		 */
		#region Shader
        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Shader_CreateShader(string filepath, out IntPtr native);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Shader_Destroy(IntPtr native);

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Shader_SetInt(IntPtr natiive, string name, ref int value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Shader_SetIntArray(IntPtr natiive, string name, int[] value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Shader_SetFloat(IntPtr natiive, string name, ref float value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Shader_SetFloat3(IntPtr natiive, string name, ref Vector3 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Shader_SetFloat4(IntPtr natiive, string name, ref Vector4 value);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Shader_Reset(IntPtr native, string filepath, out IntPtr value);
        #endregion

        /**
		 * Log
		 */
        #region Log
        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Log_Log(string message, Log.Type type);
        #endregion

        /**
		 * Renderer (for debugging)
		 */
        #region Renderer
        [MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Renderer_DrawQuad(ref Vector3 position, ref Vector3 scale, ref Vector4 colour);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Renderer_DrawQuadShader(ref Vector3 position, ref Vector3 scale, ref Vector4 colour, IntPtr shader);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Renderer_DrawRotatedQuad(ref Vector3 position, ref Vector3 scale, ref Vector3 rotation, ref Vector4 colour);
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Renderer_DrawLine(ref Vector3 p0, ref Vector3 p1, ref Vector4 colour);
        #endregion

        #region Application
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Application_Close();
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Application_GetImGuiWindowSize(string windowName, out Vector2 size);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Application_IsImGuiWindowHovered(string windowName);
        #endregion

        #region Window
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Window_GetSize(out Vector2 size);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetSize(Vector2 size);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static int Window_GetMode();
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetMode(int mode);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static string Window_GetTitle();
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetTitle(string title);
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static bool Window_GetVsync();
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static void Window_SetVsync(bool vsync);
        
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static Vector2 Window_GetPosition(out Vector2 result);
		
		[MethodImpl]
		internal extern static void Window_SetOpacity(float opacity);
        
        #endregion

        #region Framebuffer
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static uint Framebuffer_GetColorAttachmentRendererID(IntPtr nativePtr, uint index);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static IntPtr Framebuffer_Create(uint width, uint height, uint samples, FramebufferTextureFormat[] attachments, bool swapChainTarget);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Framebuffer_Invalidate(IntPtr nativePtr);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Framebuffer_Bind(IntPtr nativePtr);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Framebuffer_Unbind(IntPtr nativePtr);
        
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Framebuffer_Resize(IntPtr nativePtr, uint width, uint height);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Framebuffer_ClearAttachment(IntPtr nativePtr, uint attachmentIndex, int value);
		
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static void Framebuffer_Destroy(IntPtr nativePtr);
        #endregion

        #region Scene
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static Entity[] Scene_GetAllEntities();

		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static string Scene_LoadScene(string sceneName);
		#endregion

		#region PrefabExt
		[MethodImpl(MethodImplOptions.InternalCall)]
		internal extern static ulong PrefabExt_Instantiate(ulong prefabId, Vector3 position);
        #endregion
        
        #region Mono
        
        [MethodImpl(MethodImplOptions.InternalCall)]
        internal extern static object Mono_GetGCHandleTarget(UInt32 handle);
        
        #endregion
	}
}
