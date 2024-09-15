#pragma once
#include <string>
#include <unordered_map>
#include <filesystem>
#include <map>

#include "Buffer.h"
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

    class Shader : public RefCounted {
    public:
        Shader(const std::filesystem::path& filepath);
        Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
        virtual ~Shader();

        void bind() const;
        void unbind() const;

        void setInt(const std::string& name, int value);
        void setIntArray(const std::string& name, int* values, uint32_t count);
        void setFloat(const std::string& name, float value);
        void setFloat2(const std::string& name, const glm::vec2& value);
        void setFloat3(const std::string& name, const glm::vec3& value);
        void setFloat4(const std::string& name, const glm::vec4& value);
        void setMat3(const std::string& name, const glm::mat3& value);
        void setMat4(const std::string& name, const glm::mat4& value);

        const std::string& getName() const { return m_Name; }
        const std::filesystem::path& getFilepath() const { return filepath; }

        std::map<std::string, ShaderDataType> getActiveUniforms();

        int getInt(const std::string& name);
        float getFloat(const std::string& name);
        glm::vec2 getFloat2(const std::string& name);
        glm::vec3 getFloat3(const std::string& name);
        glm::vec4 getFloat4(const std::string& name);

        template <typename T> requires std::is_same_v<T, int> || std::is_same_v<T, float> || std::is_same_v<
            T, glm::vec2> || std::is_same_v<T, glm::vec3> || std::is_same_v<T, glm::vec4>
        void setUniformNextFrame(const std::string& name, const T& value) {
            if constexpr (std::is_same_v<T, int>) {
                m_IntNextFrame[name] = value;
            }
            else if constexpr (std::is_same_v<T, float>) {
                m_FloatNextFrame[name] = value;
            }
            else if constexpr (std::is_same_v<T, glm::vec2>) {
                m_Float2NextFrame[name] = glm::vec2(value);
            }
            else if constexpr (std::is_same_v<T, glm::vec3>) {
                m_Float3NextFrame[name] = glm::vec3(value);
            }
            else if constexpr (std::is_same_v<T, glm::vec4>) {
                m_Float4NextFrame[name] = glm::vec4(value);
            }
        }

    private:
        std::string readFile(const std::filesystem::path& filepath);
        std::unordered_map<unsigned int, std::string> preProcess(const std::string& source);
        void compile(const std::unordered_map<unsigned int, std::string>& shaderSources);

    private:
        uint32_t m_Renderer2DID;
        std::string m_Name;
        std::filesystem::path filepath;

        std::unordered_map<std::string, int> m_IntNextFrame;
        std::unordered_map<std::string, float> m_FloatNextFrame;
        std::unordered_map<std::string, glm::vec2> m_Float2NextFrame;
        std::unordered_map<std::string, glm::vec3> m_Float3NextFrame;
        std::unordered_map<std::string, glm::vec4> m_Float4NextFrame;

        friend class Renderer2D;
        friend class SceneSerializer;
    };
}
