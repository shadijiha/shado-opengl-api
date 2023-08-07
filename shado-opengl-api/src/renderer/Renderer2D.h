﻿#pragma once

#ifndef RENDERER_2D_H
#define RENDERER_2D_H

#include "util/Util.h"
#include "cameras/OrthoCamera.h"
#include "Shader.h"
#include "VertexArray.h"
#include "Texture2D.h"

namespace Shado {
	struct SpriteRendererComponent;

	inline const std::filesystem::path QUAD_SHADER = "assets/shaders/Renderer2D_Quad.glsl";
	inline const std::filesystem::path CIRCLE_SHADER = "assets/shaders/Renderer2D_Circle.glsl";
	inline const std::filesystem::path LINES_SHADER = "assets/shaders/Renderer2D_Line.glsl";

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
		static void DrawQuad(const glm::vec2& position, const glm::vec2& size, Texture2D* texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawQuad(const glm::vec3& position, const glm::vec2& size, Texture2D* texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));

		static void DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, Texture2D* texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f), int entityID = -1);
		static void DrawQuad(const glm::mat4& transform, Shader& shader, const glm::vec4& color = {1,1,1,1}, int entityID = -1);

		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec3& rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec3& rotation, const glm::vec4& color);
		static void DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec3& rotation, Texture2D* texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
		static void DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec3& rotation, Texture2D* texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f));
	
		static void DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness = 1.0f, float fade = 0.005f, int entityID = -1);
		static void DrawCircle(const glm::mat4& transform, Texture2D* texture, float tilingFactor = 1.0f, const glm::vec4& tintColor = glm::vec4(1.0f), float thickness = 1.0f, float fade = 0.005f, int entityID = -1);
		static void DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, int entityID = -1);

		static void DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID);
		static void DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID);

		static void DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID);

		static float GetLineWidth();
		static void SetLineWidth(float width);

		static void setCPUAlphaZSorting(bool b) { CPUAlphaZSorting = b; }

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
		static void CmdDrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0);
		static void CmdDrawIndexedLine(const Ref<VertexArray>& vertexArray, uint32_t indexCount = 0);
		inline static bool s_Init = false;
		inline static bool CPUAlphaZSorting = true;

	private:
		static void StartBatch();
		static void NextBatch();
	};
}

#endif
