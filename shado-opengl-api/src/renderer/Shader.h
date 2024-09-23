#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include <map>

#include "Buffer.h"
#include "asset/Asset.h"
#include "glm/glm.hpp"
#include "util/Memory.h"

namespace Shado {
    class ShaderCompilationException : public std::runtime_error {
    public:
        ShaderCompilationException(const std::string& message)
            : std::runtime_error(message) {
        }
    };

    class ShaderFileException : public std::runtime_error {
    public:
        ShaderFileException(const std::string& message)
            : std::runtime_error(message) {
        }
    };

    class Shader : public Asset {
    public:
        Shader(const std::string& fileContent);
        Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
        Shader(const Shader& other) = delete;
        virtual ~Shader();

        void bind() const;
        void unbind() const;
        void copyCustomUniformsTo(Ref<Shader>& target) const;

        void setInt(const std::string& name, int value);
        void setIntArray(const std::string& name, int* values, uint32_t count);
        void setFloat(const std::string& name, float value);
        void setFloat2(const std::string& name, const glm::vec2& value);
        void setFloat3(const std::string& name, const glm::vec3& value);
        void setFloat4(const std::string& name, const glm::vec4& value);
        void setMat3(const std::string& name, const glm::mat3& value);
        void setMat4(const std::string& name, const glm::mat4& value);

        std::map<std::string, ShaderDataType> getActiveUniforms();

        int getInt(const std::string& name);
        float getFloat(const std::string& name);
        glm::vec2 getFloat2(const std::string& name);
        glm::vec3 getFloat3(const std::string& name);
        glm::vec4 getFloat4(const std::string& name);

        template <typename T>
        void saveCustomUniformValue(const std::string& uniformName, ShaderDataType type, const T& value) {
            m_CustomUniforms[uniformName] = std::make_tuple(type, snew(T) T(value));
        }

        static AssetType GetStaticType() { return AssetType::Shader; }
        AssetType GetType() const override { return GetStaticType(); }

    private:
        std::string readFile(const std::filesystem::path& filepath);
        std::unordered_map<unsigned int, std::string> preProcess(const std::string& source);
        void compile(const std::unordered_map<unsigned int, std::string>& shaderSources);

        static int getCurrentActiveProgram();

    private:
        uint32_t m_Renderer2DID;

        // Uniforms set by the editor. They are serialized and deserialized by the SceneSerializer
        std::unordered_map<std::string, std::tuple<ShaderDataType, void*>> m_CustomUniforms;

        friend class Renderer2D;
        friend class SceneSerializer;
    };
}
