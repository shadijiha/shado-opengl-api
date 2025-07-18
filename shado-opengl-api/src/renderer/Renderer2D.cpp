﻿#include "Renderer2D.h"
#include <GL/glew.h>
#include "Buffer.h"
#include "debug/Profile.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include "cameras/OrbitCamera.h"
#include "VertexArray.h"
#include <array>

#include "UniformBuffer.h"
#include "scene/Components.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

#undef INFINITE
#include <FontGeometry.h>
#include "Events/input.h"
#include "asset/AssetManager.h"
#include "asset/Importer.h"

namespace Shado {
    struct QuadVertex {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;

        // Editor-only
        int EntityID;
    };

    struct CircleVertex {
        glm::vec3 WorldPosition;
        glm::vec3 LocalPosition;
        glm::vec4 Color;
        float Thickness;
        float Fade;

        // Editor-only
        int EntityID;

        glm::vec2 TexCoord;
        float TexIndex;
        float TilingFactor;
    };

    struct LineVertex {
        glm::vec3 Position;
        glm::vec4 Color;

        // Editor-only
        int EntityID;
    };

    struct TextVertex {
        glm::vec3 Position;
        glm::vec4 Color;
        glm::vec2 TexCoord;

        // TODO: bg color for outline/bg

        // Editor-only
        int EntityID;
    };

    struct QuadFace {
        uint32_t vertexCount = 4;
        float zBuffer;
        std::vector<QuadVertex> vertices;
    };

    struct Renderer2DData {
        static const uint32_t MaxQuads = 20000;
        static const uint32_t MaxVertices = MaxQuads * 4;
        static const uint32_t MaxIndices = MaxQuads * 6;
        static const uint32_t MaxTextureSlots = 32; // TODO: RenderCaps

        Ref<VertexArray> QuadVertexArray;
        Ref<VertexBuffer> QuadVertexBuffer;
        Ref<Shader> QuadShader;
        Ref<Texture2D> WhiteTexture;
        std::vector<QuadFace> transparentQuads;


        Ref<VertexArray> CircleVertexArray;
        Ref<VertexBuffer> CircleVertexBuffer;
        Ref<Shader> CircleShader;

        Ref<VertexArray> LineVertexArray;
        Ref<VertexBuffer> LineVertexBuffer;
        Ref<Shader> LineShader;

        Ref<VertexArray> TextVertexArray;
        Ref<VertexBuffer> TextVertexBuffer;
        Ref<Shader> TextShader;

        uint32_t QuadIndexCount = 0;
        QuadVertex* QuadVertexBufferBase = nullptr;
        QuadVertex* QuadVertexBufferPtr = nullptr;

        uint32_t CircleIndexCount = 0;
        CircleVertex* CircleVertexBufferBase = nullptr;
        CircleVertex* CircleVertexBufferPtr = nullptr;

        uint32_t LineVertexCount = 0;
        LineVertex* LineVertexBufferBase = nullptr;
        LineVertex* LineVertexBufferPtr = nullptr;

        uint32_t TextIndexCount = 0;
        TextVertex* TextVertexBufferBase = nullptr;
        TextVertex* TextVertexBufferPtr = nullptr;

        float LineWidth = 2.0f;

        std::array<Ref<Texture2D>, MaxTextureSlots> TextureSlots;
        uint32_t TextureSlotIndex = 1; // 0 = white texture

        Ref<Texture2D> FontAtlasTexture;

        glm::vec4 QuadVertexPositions[4];

        Renderer2D::Statistics Stats;

        struct CameraData {
            glm::mat4 ViewProjection;
        };

        CameraData CameraBuffer;
        Ref<UniformBuffer> CameraUniformBuffer;
    };

    static Renderer2DData s_Data;

    void Renderer2D::Init() {
        SHADO_PROFILE_FUNCTION();

        s_Init = true;

        s_Data.QuadVertexArray = VertexArray::create();

        s_Data.QuadVertexBuffer = VertexBuffer::create(s_Data.MaxVertices * sizeof(QuadVertex));
        s_Data.QuadVertexBuffer->setLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float, "a_TexIndex"},
            {ShaderDataType::Float, "a_TilingFactor"},
            {ShaderDataType::Int, "a_EntityID"}
        });
        s_Data.QuadVertexArray->addVertexBuffer(s_Data.QuadVertexBuffer);

        s_Data.QuadVertexBufferBase = Memory::Heap<QuadVertex>(s_Data.MaxVertices, "Renderer2D");

        uint32_t* quadIndices = Memory::Heap<uint32_t>(s_Data.MaxIndices, "Renderer2D");

        uint32_t offset = 0;
        for (uint32_t i = 0; i < s_Data.MaxIndices; i += 6) {
            quadIndices[i + 0] = offset + 0;
            quadIndices[i + 1] = offset + 1;
            quadIndices[i + 2] = offset + 2;

            quadIndices[i + 3] = offset + 2;
            quadIndices[i + 4] = offset + 3;
            quadIndices[i + 5] = offset + 0;

            offset += 4;
        }

        Ref<IndexBuffer> quadIB = IndexBuffer::create(quadIndices, s_Data.MaxIndices);
        s_Data.QuadVertexArray->setIndexBuffer(quadIB);
        Memory::Free(quadIndices);

        // Circles
        s_Data.CircleVertexArray = VertexArray::create();

        s_Data.CircleVertexBuffer = VertexBuffer::create(s_Data.MaxVertices * sizeof(CircleVertex));
        s_Data.CircleVertexBuffer->setLayout({
            {ShaderDataType::Float3, "a_WorldPosition"},
            {ShaderDataType::Float3, "a_LocalPosition"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Float, "a_Thickness"},
            {ShaderDataType::Float, "a_Fade"},
            {ShaderDataType::Int, "a_EntityID"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float, "a_TexIndex"},
            {ShaderDataType::Float, "a_TilingFactor"}
        });
        s_Data.CircleVertexArray->addVertexBuffer(s_Data.CircleVertexBuffer);
        s_Data.CircleVertexArray->setIndexBuffer(quadIB); // Use quad IB
        s_Data.CircleVertexBufferBase = Memory::Heap<CircleVertex>(s_Data.MaxVertices);

        // Lines
        s_Data.LineVertexArray = VertexArray::create();

        s_Data.LineVertexBuffer = VertexBuffer::create(s_Data.MaxVertices * sizeof(LineVertex));
        s_Data.LineVertexBuffer->setLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Int, "a_EntityID"}
        });
        s_Data.LineVertexArray->addVertexBuffer(s_Data.LineVertexBuffer);
        s_Data.LineVertexBufferBase = Memory::Heap<LineVertex>(s_Data.MaxVertices, "Renderer2D");

        // Text
        s_Data.TextVertexArray = VertexArray::create();
        s_Data.TextVertexBuffer = VertexBuffer::create(s_Data.MaxVertices * sizeof(TextVertex));
        s_Data.TextVertexBuffer->setLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Int, "a_EntityID"}
        });
        s_Data.TextVertexArray->addVertexBuffer(s_Data.TextVertexBuffer);
        s_Data.TextVertexArray->setIndexBuffer(quadIB);
        s_Data.TextVertexBufferBase = new TextVertex[s_Data.MaxVertices];


        s_Data.WhiteTexture = snew(Texture2D) Texture2D(1, 1);
        uint32_t whiteTextureData = 0xffffffff;
        s_Data.WhiteTexture->setData(Buffer(&whiteTextureData, sizeof(uint32_t)));

        int32_t samplers[s_Data.MaxTextureSlots];
        for (uint32_t i = 0; i < s_Data.MaxTextureSlots; i++)
            samplers[i] = i;

        s_Data.QuadShader = ShaderImporter::LoadShader("assets/shaders/Renderer2D_Quad.glsl");
        s_Data.CircleShader = ShaderImporter::LoadShader("assets/shaders/Renderer2D_Circle.glsl");
        s_Data.LineShader = ShaderImporter::LoadShader("assets/shaders/Renderer2D_Line.glsl");
        s_Data.TextShader = ShaderImporter::LoadShader("assets/shaders/Renderer2D_Text.glsl");

        // Set first texture slot to 0
        s_Data.TextureSlots[0] = s_Data.WhiteTexture;

        s_Data.QuadVertexPositions[0] = {-0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[1] = {0.5f, -0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[2] = {0.5f, 0.5f, 0.0f, 1.0f};
        s_Data.QuadVertexPositions[3] = {-0.5f, 0.5f, 0.0f, 1.0f};

        s_Data.CameraUniformBuffer = UniformBuffer::create(sizeof(Renderer2DData::CameraData), 0);

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LINE_SMOOTH);
        glDepthFunc(GL_LESS);
        glEnable(GL_ALPHA_TEST);
        glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);

        /*
         * Debug
         */
#if SHADO_DEBUG || SHADO_RELEASE
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); // Ensures the callback is executed immediately
        glDebugMessageCallback([](GLenum source, GLenum type, GLuint id,
                                    GLenum severity, GLsizei length,
                                    const GLchar *message, const void* userParam) {
                std::string severityString;
                switch (severity) {
                    case GL_DEBUG_SEVERITY_HIGH:
                        severityString = "SEVERE ERROR";
                        break;
                    case GL_DEBUG_SEVERITY_MEDIUM:
                        severityString = "Medium severity";
                        break;
                    default:
                        return;
                }
                SHADO_CORE_ERROR("[OpenGL Error] {} ({}): {}", severityString, id, message);
            }, nullptr);
        glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
#endif
    }

    void Renderer2D::Shutdown() {
        SHADO_PROFILE_FUNCTION();

        Memory::Free(s_Data.QuadVertexBufferBase);
    }

    void Renderer2D::BeginScene(const Camera& camera) {
        SHADO_PROFILE_FUNCTION();

        s_Data.CameraBuffer.ViewProjection = camera.getViewProjectionMatrix();
        s_Data.CameraUniformBuffer->setData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

        StartBatch();
    }

    void Renderer2D::BeginScene(const Camera& camera, const glm::mat4& transform) {
        SHADO_PROFILE_FUNCTION();

        s_Data.CameraBuffer.ViewProjection = camera.getProjectionMatrix() * glm::inverse(transform);
        s_Data.CameraUniformBuffer->setData(&s_Data.CameraBuffer, sizeof(Renderer2DData::CameraData));

        StartBatch();
    }

    void Renderer2D::EndScene() {
        SHADO_PROFILE_FUNCTION();

        Flush();
    }

    void Renderer2D::StartBatch() {
        SHADO_PROFILE_FUNCTION();

        s_Data.QuadIndexCount = 0;
        s_Data.QuadVertexBufferPtr = s_Data.QuadVertexBufferBase;

        s_Data.CircleIndexCount = 0;
        s_Data.CircleVertexBufferPtr = s_Data.CircleVertexBufferBase;

        s_Data.LineVertexCount = 0;
        s_Data.LineVertexBufferPtr = s_Data.LineVertexBufferBase;

        s_Data.TextIndexCount = 0;
        s_Data.TextVertexBufferPtr = s_Data.TextVertexBufferBase;

        s_Data.TextureSlotIndex = 1;
    }

    void Renderer2D::Flush() {
        if (s_Data.QuadIndexCount) {
            // This is here because when batch rendering, OpenGL depth test does not work
            // as intended
            if (CPUAlphaZSorting) {
                // Sort transparent objects by Z
                std::sort(s_Data.transparentQuads.begin(), s_Data.transparentQuads.end(), [](QuadFace& a, QuadFace& b) {
                    return a.zBuffer < b.zBuffer;
                });
                // Add the transparent objects to the end of the array
                for (const auto& face : s_Data.transparentQuads) {
                    for (size_t i = 0; i < face.vertexCount; i++) {
                        *s_Data.QuadVertexBufferPtr = face.vertices[i];
                        s_Data.QuadVertexBufferPtr++;
                    }
                }
                s_Data.transparentQuads.clear();
            }

            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.QuadVertexBufferPtr - (uint8_t*)s_Data.
                QuadVertexBufferBase);
            s_Data.QuadVertexBuffer->setData(s_Data.QuadVertexBufferBase, dataSize);

            // Bind textures
            for (uint32_t i = 0; i < s_Data.TextureSlotIndex; i++)
                s_Data.TextureSlots[i]->bind(i);

            s_Data.QuadShader->bind();
            s_Data.QuadShader->setFloat("u_Time", (float)glfwGetTime());
            s_Data.QuadShader->setFloat2("u_ScreenResolution", {
                                             Application::get().getWindow().getWidth(),
                                             Application::get().getWindow().getHeight()
                                         });
            s_Data.QuadShader->setFloat2("u_MousePos", {Input::getMouseX(), Input::getMouseY()});
            CmdDrawIndexed(s_Data.QuadVertexArray, s_Data.QuadIndexCount);
            s_Data.Stats.DrawCalls++;
        }

        if (s_Data.CircleIndexCount) {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.CircleVertexBufferPtr - (uint8_t*)s_Data.
                CircleVertexBufferBase);
            s_Data.CircleVertexBuffer->setData(s_Data.CircleVertexBufferBase, dataSize);

            s_Data.CircleShader->bind();
            CmdDrawIndexed(s_Data.CircleVertexArray, s_Data.CircleIndexCount);
            s_Data.Stats.DrawCalls++;
        }

        if (s_Data.LineVertexCount) {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.LineVertexBufferPtr - (uint8_t*)s_Data.
                LineVertexBufferBase);
            s_Data.LineVertexBuffer->setData(s_Data.LineVertexBufferBase, dataSize);

            s_Data.LineShader->bind();
            //RenderCommand::SetLineWidth(s_Data.LineWidth);
            CmdDrawIndexedLine(s_Data.LineVertexArray, s_Data.LineVertexCount);
            s_Data.Stats.DrawCalls++;
        }

        if (s_Data.TextIndexCount) {
            uint32_t dataSize = (uint32_t)((uint8_t*)s_Data.TextVertexBufferPtr - (uint8_t*)s_Data.
                TextVertexBufferBase);
            s_Data.TextVertexBuffer->setData(s_Data.TextVertexBufferBase, dataSize);

            auto buf = s_Data.TextVertexBufferBase;
            s_Data.FontAtlasTexture->bind(0);

            s_Data.TextShader->bind();
            CmdDrawIndexed(s_Data.TextVertexArray, s_Data.TextIndexCount);
            s_Data.Stats.DrawCalls++;

            // Rebind white texture
            s_Data.WhiteTexture->bind(0);
        }
    }

    void Renderer2D::SetClearColor(const glm::vec4& color) {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void Renderer2D::NextBatch() {
        Flush();
        StartBatch();
    }

    void Renderer2D::Clear() {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& color) {
        DrawQuad({position.x, position.y, 0.0f}, size, color);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, color);
    }

    void Renderer2D::DrawQuad(const glm::vec2& position, const glm::vec2& size, AssetHandle textureHandle,
                              float tilingFactor, const glm::vec4& tintColor) {
        DrawQuad({position.x, position.y, 0.0f}, size, textureHandle, tilingFactor, tintColor);
    }

    void Renderer2D::DrawQuad(const glm::vec3& position, const glm::vec2& size, AssetHandle textureHandlee,
                              float tilingFactor, const glm::vec4& tintColor) {
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, textureHandlee, tilingFactor, tintColor);
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, const glm::vec4& color, int entityID) {
        SHADO_PROFILE_FUNCTION();

        constexpr size_t quadVertexCount = 4;
        constexpr float textureIndex = 0.0f; // White Texture
        constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
        constexpr float tilingFactor = 1.0f;

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        QuadVertex temp[quadVertexCount];
        for (size_t i = 0; i < quadVertexCount; i++) {
            temp[i].Position = transform * s_Data.QuadVertexPositions[i];
            temp[i].Color = color;
            temp[i].TexCoord = textureCoords[i];
            temp[i].TexIndex = textureIndex;
            temp[i].TilingFactor = tilingFactor;
            temp[i].EntityID = entityID;
        }

        // If entity is transparent, Draw it later
        QuadFace face;
        for (size_t i = 0; i < quadVertexCount; i++) {
            if (CPUAlphaZSorting && color.a < 1.0f) {
                face.vertices.push_back(temp[i]);
            }
            else {
                *s_Data.QuadVertexBufferPtr = temp[i];
                s_Data.QuadVertexBufferPtr++;
            }
        }

        // Again check if there's an alpha
        // if yes, don't add the quad now, wait until the Flush so the alpha quads are drawn on top of everything
        if (CPUAlphaZSorting && color.a < 1.0f) {
            face.zBuffer = transform[3].z;
            s_Data.transparentQuads.push_back(face);
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, AssetHandle textureHandle, float tilingFactor,
                              const glm::vec4& tintColor, int entityID) {
        SHADO_PROFILE_FUNCTION();

        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
        Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureHandle);

        if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
            NextBatch();

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
            if (*s_Data.TextureSlots[i] == *texture) {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f) {
            if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                NextBatch();

            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        QuadVertex temp[quadVertexCount];
        for (size_t i = 0; i < quadVertexCount; i++) {
            temp[i].Position = transform * s_Data.QuadVertexPositions[i];
            temp[i].Color = tintColor;
            temp[i].TexCoord = textureCoords[i];
            temp[i].TexIndex = textureIndex;
            temp[i].TilingFactor = tilingFactor;
            temp[i].EntityID = entityID;
        }

        // If entity is transparent, Draw it later
        QuadFace face;
        for (size_t i = 0; i < quadVertexCount; i++) {
            if (CPUAlphaZSorting && tintColor.a < 1.0f) {
                face.vertices.push_back(temp[i]);
            }
            else {
                *s_Data.QuadVertexBufferPtr = temp[i];
                s_Data.QuadVertexBufferPtr++;
            }
        }

        // Again check if there's an alpha
        // if yes, don't add the quad now, wait until the Flush so the alpha quads are drawn on top of everything
        if (CPUAlphaZSorting && tintColor.a < 1.0f) {
            face.zBuffer = transform[3].z;
            s_Data.transparentQuads.push_back(face);
        }

        s_Data.QuadIndexCount += 6;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, AssetHandle shaderHandle, const glm::vec4& color,
                              int entityID,
                              float textureIndex) {
        SHADO_PROFILE_FUNCTION();
        Ref<Shader> shader = AssetManager::GetAsset<Shader>(shaderHandle);
        shader->bind();

        uint32_t quadIndices[6];
        quadIndices[0] = 0;
        quadIndices[1] = 1;
        quadIndices[2] = 2;
        quadIndices[3] = 2;
        quadIndices[4] = 3;
        quadIndices[5] = 0;

        Ref<VertexArray> vertexArray = CreateRef<VertexArray>();
        Ref<VertexBuffer> buffer = VertexBuffer::create(4 * sizeof(QuadVertex));
        buffer->setLayout({
            {ShaderDataType::Float3, "a_Position"},
            {ShaderDataType::Float4, "a_Color"},
            {ShaderDataType::Float2, "a_TexCoord"},
            {ShaderDataType::Float, "a_TexIndex"},
            {ShaderDataType::Float, "a_TilingFactor"},
            {ShaderDataType::Int, "a_EntityID"}
        });
        vertexArray->addVertexBuffer(buffer);
        Ref<IndexBuffer> indexBuffer = CreateRef<IndexBuffer>(quadIndices, 6);
        vertexArray->setIndexBuffer(indexBuffer);

        constexpr size_t quadVertexCount = 4;
        constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

        QuadVertex vertex[4];
        for (size_t i = 0; i < quadVertexCount; i++) {
            vertex[i].Position = transform * s_Data.QuadVertexPositions[i];
            vertex[i].Color = color;
            vertex[i].TexCoord = textureCoords[i];
            vertex[i].TexIndex = textureIndex;
            vertex[i].TilingFactor = 1.0f;
            vertex[i].EntityID = entityID;
        }

        buffer->setData(&vertex, 4 * sizeof(QuadVertex));

        // if transparent, then draw 
        if (color.a < 1.0f) {
            NextBatch();
        }

        shader->bind();
        shader->setFloat("u_Time", (float)glfwGetTime());

        auto res = glm::vec2{Application::get().getWindow().getWidth(), Application::get().getWindow().getHeight()};
        shader->setFloat2("u_ScreenResolution", res);
        shader->setFloat2("u_MousePos", {Input::getMouseX(), Input::getMouseY()});

        CmdDrawIndexed(vertexArray, indexBuffer->getCount());
        s_Data.Stats.DrawCalls++;
        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawQuad(const glm::mat4& transform, AssetHandle textureHandle, AssetHandle shaderHandle,
                              const glm::vec4& color, int entityID) {
        SHADO_PROFILE_FUNCTION();
        Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureHandle);
        
        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
            if (*s_Data.TextureSlots[i] == *texture) {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f) {
            if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                NextBatch();

            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        texture->bind(textureIndex);
        DrawQuad(transform, shaderHandle, color, entityID, textureIndex);
        texture->unbind();
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec3& rotation,
                                     const glm::vec4& color) {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec3& rotation,
                                     const glm::vec4& color) {
        SHADO_PROFILE_FUNCTION();

        glm::mat4 _rotation = glm::toMat4(glm::quat(rotation));
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * _rotation
            * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, color);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec3& rotation,
                                     AssetHandle textureHandle, float tilingFactor, const glm::vec4& tintColor) {
        DrawRotatedQuad({position.x, position.y, 0.0f}, size, rotation, textureHandle, tilingFactor, tintColor);
    }

    void Renderer2D::DrawRotatedQuad(const glm::vec3& position, const glm::vec2& size, const glm::vec3& rotation,
                                     AssetHandle textureHandle, float tilingFactor, const glm::vec4& tintColor) {
        SHADO_PROFILE_FUNCTION();

        glm::mat4 _rotation = glm::toMat4(glm::quat(rotation));
        glm::mat4 transform = glm::translate(glm::mat4(1.0f), position)
            * _rotation
            * glm::scale(glm::mat4(1.0f), {size.x, size.y, 1.0f});

        DrawQuad(transform, textureHandle, tilingFactor, tintColor);
    }

    void Renderer2D::DrawCircle(const glm::mat4& transform, const glm::vec4& color, float thickness /*= 1.0f*/,
                                float fade /*= 0.005f*/, int entityID /*= -1*/) {
        SHADO_PROFILE_FUNCTION();
        // TODO: implement for circles
        // if (s_Data.QuadIndexCount >= Renderer2DData::MaxIndices)
        // 	NextBatch();
        constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};

        for (size_t i = 0; i < 4; i++) {
            s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPositions[i];
            s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
            s_Data.CircleVertexBufferPtr->Color = color;
            s_Data.CircleVertexBufferPtr->Thickness = thickness;
            s_Data.CircleVertexBufferPtr->Fade = fade;
            s_Data.CircleVertexBufferPtr->EntityID = entityID;
            s_Data.CircleVertexBufferPtr->TexIndex = 0.0f;
            s_Data.CircleVertexBufferPtr->TilingFactor = 1.0f;
            s_Data.CircleVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.CircleVertexBufferPtr++;
        }

        s_Data.CircleIndexCount += 6;

        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawCircle(const glm::mat4& transform, AssetHandle textureHandle, float tilingFactor,
                                const glm::vec4& tintColor, float thickness, float fade, int entityID) {
        SHADO_PROFILE_FUNCTION();

        constexpr glm::vec2 textureCoords[] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}, {0.0f, 1.0f}};
        Ref<Texture2D> texture = AssetManager::GetAsset<Texture2D>(textureHandle);

        float textureIndex = 0.0f;
        for (uint32_t i = 1; i < s_Data.TextureSlotIndex; i++) {
            if (*s_Data.TextureSlots[i] == *texture) {
                textureIndex = (float)i;
                break;
            }
        }

        if (textureIndex == 0.0f) {
            if (s_Data.TextureSlotIndex >= Renderer2DData::MaxTextureSlots)
                NextBatch();

            textureIndex = (float)s_Data.TextureSlotIndex;
            s_Data.TextureSlots[s_Data.TextureSlotIndex] = texture;
            s_Data.TextureSlotIndex++;
        }

        for (size_t i = 0; i < 4; i++) {
            s_Data.CircleVertexBufferPtr->WorldPosition = transform * s_Data.QuadVertexPositions[i];
            s_Data.CircleVertexBufferPtr->LocalPosition = s_Data.QuadVertexPositions[i] * 2.0f;
            s_Data.CircleVertexBufferPtr->Color = tintColor;
            s_Data.CircleVertexBufferPtr->Thickness = thickness;
            s_Data.CircleVertexBufferPtr->Fade = fade;
            s_Data.CircleVertexBufferPtr->EntityID = entityID;

            s_Data.CircleVertexBufferPtr->TexCoord = textureCoords[i];
            s_Data.CircleVertexBufferPtr->TexIndex = textureIndex;
            s_Data.CircleVertexBufferPtr->TilingFactor = tilingFactor;
            s_Data.CircleVertexBufferPtr++;
        }

        s_Data.CircleIndexCount += 6;

        s_Data.Stats.QuadCount++;
    }

    void Renderer2D::DrawLine(const glm::vec3& p0, const glm::vec3& p1, const glm::vec4& color, int entityID) {
        SHADO_PROFILE_FUNCTION();

        s_Data.LineVertexBufferPtr->Position = p0;
        s_Data.LineVertexBufferPtr->Color = color;
        s_Data.LineVertexBufferPtr->EntityID = entityID;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexBufferPtr->Position = p1;
        s_Data.LineVertexBufferPtr->Color = color;
        s_Data.LineVertexBufferPtr->EntityID = entityID;
        s_Data.LineVertexBufferPtr++;

        s_Data.LineVertexCount += 2;
    }

    void Renderer2D::DrawRect(const glm::vec3& position, const glm::vec2& size, const glm::vec4& color, int entityID) {
        SHADO_PROFILE_FUNCTION();

        glm::vec3 p0 = glm::vec3(position.x - size.x * 0.5f, position.y - size.y * 0.5f, position.z);
        glm::vec3 p1 = glm::vec3(position.x + size.x * 0.5f, position.y - size.y * 0.5f, position.z);
        glm::vec3 p2 = glm::vec3(position.x + size.x * 0.5f, position.y + size.y * 0.5f, position.z);
        glm::vec3 p3 = glm::vec3(position.x - size.x * 0.5f, position.y + size.y * 0.5f, position.z);

        DrawLine(p0, p1, color);
        DrawLine(p1, p2, color);
        DrawLine(p2, p3, color);
        DrawLine(p3, p0, color);
    }

    void Renderer2D::DrawRect(const glm::mat4& transform, const glm::vec4& color, int entityID) {
        SHADO_PROFILE_FUNCTION();

        glm::vec3 lineVertices[4];
        for (size_t i = 0; i < 4; i++)
            lineVertices[i] = transform * s_Data.QuadVertexPositions[i];

        DrawLine(lineVertices[0], lineVertices[1], color);
        DrawLine(lineVertices[1], lineVertices[2], color);
        DrawLine(lineVertices[2], lineVertices[3], color);
        DrawLine(lineVertices[3], lineVertices[0], color);
    }

    void Renderer2D::DrawSprite(const glm::mat4& transform, SpriteRendererComponent& src, int entityID) {
        if (src.shader) {
            if (src.texture) {
                DrawQuad(transform, src.texture, src.shader, src.color, entityID);
            }
            else
                DrawQuad(transform, src.shader, src.color, entityID);
        }
        else {
            if (src.texture) {
                DrawQuad(transform, src.texture, src.tilingFactor, src.color, entityID);
            }
            else
                DrawQuad(transform, src.color, entityID);
        }
    }

    void Renderer2D::DrawString(const glm::mat4& transform, const TextComponent& textRenderer,
                                int entityID) {
        if (!textRenderer.font || textRenderer.text.empty())
            return;

        auto& font = textRenderer.font;
        auto& string = textRenderer.text;

        const auto& fontGeometry = font->getData().m_FontGeometry;
        const auto& metrics = fontGeometry->getMetrics();
        Ref<Texture2D> fontAtlas = font->getAtlasTexture();

        s_Data.FontAtlasTexture = fontAtlas;

        double x = 0.0;
        double fsScale = 1.0 / (metrics.ascenderY - metrics.descenderY);
        double y = 0.0;

        const float spaceGlyphAdvance = fontGeometry->getGlyph(' ')->getAdvance();

        for (size_t i = 0; i < string.size(); i++) {
            auto character = string[i];
            if (character == '\r')
                continue;

            if (character == '\n') {
                x = 0;
                y -= fsScale * metrics.lineHeight + textRenderer.lineSpacing;
                continue;
            }

            if (character == ' ') {
                float advance = spaceGlyphAdvance;
                if (i < string.size() - 1) {
                    auto nextCharacter = string[i + 1];
                    double dAdvance;
                    fontGeometry->getAdvance(dAdvance, character, nextCharacter);
                    advance = (float)dAdvance;
                }

                x += fsScale * advance + textRenderer.kerning;
                continue;
            }

            if (character == '\t') {
                // NOTE(Yan): is this right?
                x += 4.0f * (fsScale * spaceGlyphAdvance + textRenderer.kerning);
                continue;
            }

            auto glyph = fontGeometry->getGlyph(character);
            if (!glyph)
                glyph = fontGeometry->getGlyph('?');
            if (!glyph)
                return;

            double al, ab, ar, at;
            glyph->getQuadAtlasBounds(al, ab, ar, at);
            glm::vec2 texCoordMin((float)al, (float)ab);
            glm::vec2 texCoordMax((float)ar, (float)at);

            double pl, pb, pr, pt;
            glyph->getQuadPlaneBounds(pl, pb, pr, pt);
            glm::vec2 quadMin((float)pl, (float)pb);
            glm::vec2 quadMax((float)pr, (float)pt);

            quadMin *= fsScale, quadMax *= fsScale;
            quadMin += glm::vec2(x, y);
            quadMax += glm::vec2(x, y);

            float texelWidth = 1.0f / fontAtlas->getWidth();
            float texelHeight = 1.0f / fontAtlas->getHeight();
            texCoordMin *= glm::vec2(texelWidth, texelHeight);
            texCoordMax *= glm::vec2(texelWidth, texelHeight);

            // render here
            s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin, 0.0f, 1.0f);
            s_Data.TextVertexBufferPtr->Color = textRenderer.color;
            s_Data.TextVertexBufferPtr->TexCoord = texCoordMin;
            s_Data.TextVertexBufferPtr->EntityID = entityID;
            s_Data.TextVertexBufferPtr++;

            s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMin.x, quadMax.y, 0.0f, 1.0f);
            s_Data.TextVertexBufferPtr->Color = textRenderer.color;
            s_Data.TextVertexBufferPtr->TexCoord = {texCoordMin.x, texCoordMax.y};
            s_Data.TextVertexBufferPtr->EntityID = entityID;
            s_Data.TextVertexBufferPtr++;

            s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax, 0.0f, 1.0f);
            s_Data.TextVertexBufferPtr->Color = textRenderer.color;
            s_Data.TextVertexBufferPtr->TexCoord = texCoordMax;
            s_Data.TextVertexBufferPtr->EntityID = entityID;
            s_Data.TextVertexBufferPtr++;

            s_Data.TextVertexBufferPtr->Position = transform * glm::vec4(quadMax.x, quadMin.y, 0.0f, 1.0f);
            s_Data.TextVertexBufferPtr->Color = textRenderer.color;
            s_Data.TextVertexBufferPtr->TexCoord = {texCoordMax.x, texCoordMin.y};
            s_Data.TextVertexBufferPtr->EntityID = entityID;
            s_Data.TextVertexBufferPtr++;

            s_Data.TextIndexCount += 6;
            s_Data.Stats.QuadCount++;

            if (i < string.size() - 1) {
                double advance = glyph->getAdvance();
                auto nextCharacter = string[i + 1];
                fontGeometry->getAdvance(advance, character, nextCharacter);

                x += fsScale * advance + textRenderer.kerning;
            }
        }
    }

    float Renderer2D::GetLineWidth() {
        return s_Data.LineWidth;
    }

    void Renderer2D::SetLineWidth(float width) {
        s_Data.LineWidth = width;
        glLineWidth(width);
    }

    void Renderer2D::ResetStats() {
        std::memset(&s_Data.Stats, 0, sizeof(Statistics));
    }

    Renderer2D::Statistics Renderer2D::GetStats() {
        return s_Data.Stats;
    }

    void Renderer2D::CmdDrawIndexed(const Ref<VertexArray>& vertexArray, uint32_t indexCount) {
        SHADO_PROFILE_FUNCTION();

        vertexArray->bind();
        uint32_t count = indexCount ? indexCount : vertexArray->getIndexBuffers()->getCount();
        glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_INT, nullptr);
    }

    void Renderer2D::CmdDrawIndexedLine(const Ref<VertexArray>& vertexArray, uint32_t vertexCount) {
        SHADO_PROFILE_FUNCTION();

        vertexArray->bind();
        glDrawArrays(GL_LINES, 0, vertexCount);
    }
}
