#pragma once

#ifndef RENDERER_2D_H
#define RENDERER_2D_H

#include "util/Util.h"
#include <memory>
#include <unordered_map>
#include "cameras/OrthoCamera.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture2D.h"

namespace Shado {

	inline std::string FLAT_COLOR_SHADER_PATH = FILE_PATH + "\\assets\\FlatColorShader.glsl";
	inline std::string TEXTURE2D_SHADER_PATH = FILE_PATH + "\\assets\\TextureShader.glsl";
	inline std::string LINES_SHADER_PATH = FILE_PATH + "\\assets\\Renderer2D_Lines.glsl";
	inline std::string CIRCLE_SHADER_PATH = FILE_PATH + "\\assets\\Renderer2D_Circles.glsl";

	class Renderer2D
	{
	public:
		static void Init();
		static void Shutdown();

		static void BeginScene(const Camera& camera, const glm::mat4& transform);
		static void BeginScene(const Camera& camera); // TODO: Remove
		static void EndScene();
		static void Flush();
		static void SetClearColor(const glm::vec4& color);
		static void Clear();

		// Primitives
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color);
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, Ref<Texture2D> texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, Ref<Texture2D> texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color);
		static void DrawQuad(const glm::mat4& transform, Ref<Texture2D> texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec3& rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, float rotation, Ref<Texture2D> texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, float rotation, Ref<Texture2D> texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec3& rotation, Ref<Texture2D> texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		static void SetLineThickness(float thickness);
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color = glm::vec4(1.0f));

		static void DrawCircle(const glm::vec3& position, const glm::vec2& size, const Color& color, float thickness = 1.0f, float fade = 0.005f);
		static void DrawCircle(const glm::mat4& transform, const Color& color, float thickness = 1.0f, float fade = 0.005f);
		
		static bool hasInitialized() { return s_Init; }

		// Stats
		struct Statistics
		{
			uint32_t DrawCalls = 0;
			uint32_t QuadCount = 0;
			uint32_t LineCount = 0;

			uint32_t GetTotalVertexCount() { return QuadCount * 4 + LineCount * 2; }
			uint32_t GetTotalIndexCount() { return QuadCount * 6 + LineCount * 2; }
		};
		static void ResetStats();
		static Statistics GetStats();
	private:
		static void FlushAndReset();
		static void FlushAndResetLines();
		static void CmdDrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0);
		static void CmdDrawIndexedLine(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0);
		static bool s_Init;
	};
}

#endif
