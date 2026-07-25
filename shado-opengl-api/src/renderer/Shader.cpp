#include "Shader.h"
#include <GL/glew.h>
#include <fstream>
#include <array>
#if defined(SHADO_PLATFORM_MACOS)
#include <regex>
#endif

#include "debug/Debug.h"
#include "glm/gtc/type_ptr.hpp"

namespace Shado {
#if defined(SHADO_PLATFORM_MACOS)
    // Apple caps OpenGL/GLSL at 4.1. The engine's shaders are authored against
    // GLSL 4.5 (#version 450 + explicit `binding=` layout qualifiers on uniform
    // blocks and samplers, which require GLSL 4.2+). Rewrite the source to
    // GLSL 4.10 on the fly (macOS only): downgrade the #version and strip
    // `binding=` qualifiers. Uniform-block and sampler bindings are instead
    // assigned from C++ (uniform blocks default to binding 0; sampler arrays
    // are set via glUniform1iv in the renderer). Windows/Linux are unaffected.
    static std::string PatchGLSLForGL41(std::string src) {
        // #version 4x0 (420..460) -> 410, keeping any trailing "core".
        // NB: use a literal replacement (no capture group) because "$1" followed
        // by digits would be misparsed as a higher-numbered group reference.
        src = std::regex_replace(src, std::regex(R"(#version\s+4[2-6]0)"), "#version 410");

        // Remove a standalone `layout(binding = N)`.
        src = std::regex_replace(src, std::regex(R"(layout\s*\(\s*binding\s*=\s*\d+\s*\)\s*)"), "");
        // Remove `, binding = N` inside a layout list.
        src = std::regex_replace(src, std::regex(R"(\s*,\s*binding\s*=\s*\d+)"), "");
        // Remove `binding = N,` when it is the first qualifier in a list.
        src = std::regex_replace(src, std::regex(R"(binding\s*=\s*\d+\s*,\s*)"), "");

        // GLSL 410 requires the interpolation qualifier ("flat") to precede the
        // storage qualifier ("in"/"out"); GLSL 420+ relaxed this. Reorder
        // "in flat"/"out flat" -> "flat in"/"flat out".
        src = std::regex_replace(src, std::regex(R"(\b(in|out)\s+flat\b)"), "flat $1");
        return src;
    }
#endif

    static GLenum ShaderTypeFromString(const std::string& type) {
        if (type == "vertex")
            return GL_VERTEX_SHADER;
        if (type == "fragment" || type == "pixel")
            return GL_FRAGMENT_SHADER;
        return 0;
    }

    Shader::Shader(const std::string& fileContent) {
        // Shader sometimes crash because of std::async in ContentBrowserPanel, so we'll make it thread safe
        static std::mutex s_Mutex;
        std::lock_guard<std::mutex> lock(s_Mutex);

        const std::string& source = fileContent;
        auto shaderSources = preProcess(source);
        compile(shaderSources);

        // See if the vertex Shader constains the basic uniforms	
        std::vector<std::string> requiredUniforms{"u_ViewProjection", "u_Transform", "a_Position"};
        for (const std::string& uni : requiredUniforms) {
            if (source.find(uni) == std::string::npos) {
                // SHADO_CORE_WARN("{0} shader does not contain the following uniform/layout: {1}", m_Name,  uni);
                // Logger is not working properly
            }
        }
    }

    Shader::Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc) {
        std::unordered_map<GLenum, std::string> sources;
        sources[GL_VERTEX_SHADER] = vertexSrc;
        sources[GL_FRAGMENT_SHADER] = fragmentSrc;
        compile(sources);
    }

    Shader::~Shader() {
        glDeleteProgram(m_Renderer2DID);

        // delete custom uniforms
        for (auto& [name, value] : m_CustomUniforms) {
            auto [type, data] = value;
            delete data;
        }
    }

    std::string Shader::readFile(const std::filesystem::path& filepath) {
        std::string result;
        std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
        if (in) {
            in.seekg(0, std::ios::end);
            size_t size = in.tellg();
            if (size != -1) {
                result.resize(size);
                in.seekg(0, std::ios::beg);
                in.read(&result[0], size);
            }
            else {
                throw ShaderFileException("Could not read from file " + filepath.string());
            }
        }
        else {
            throw ShaderFileException("Could not open file " + filepath.string());
        }

        return result;
    }

    std::unordered_map<GLenum, std::string> Shader::preProcess(const std::string& source) {
        std::unordered_map<GLenum, std::string> shaderSources;

        const char* typeToken = "#type";
        size_t typeTokenLength = strlen(typeToken);
        size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
        while (pos != std::string::npos) {
            size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
            if (eol == std::string::npos)
                throw ShaderCompilationException("Syntax error");

            size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
            std::string type = source.substr(begin, eol - begin);
            if (!ShaderTypeFromString(type))
                throw ShaderCompilationException("Invalid shader type specified");

            size_t nextLinePos = source.find_first_not_of("\r\n", eol);
            //Start of shader code after shader type declaration line
            if (nextLinePos == std::string::npos)
                throw ShaderCompilationException("Syntax error");
            pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

            shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos)
                                                            ? source.substr(nextLinePos)
                                                            : source.substr(nextLinePos, pos - nextLinePos);
        }

        return shaderSources;
    }

    void Shader::compile(const std::unordered_map<GLenum, std::string>& shaderSources) {
        GLuint program = glCreateProgram();
        if (shaderSources.size() > 2)
            throw ShaderCompilationException("We only support 2 shaders for now");

        std::array<GLenum, 2> glShaderIDs;
        int glShaderIDIndex = 0;
        for (auto& kv : shaderSources) {
            GLenum type = kv.first;
            std::string source = kv.second;

#if defined(SHADO_PLATFORM_MACOS)
            // Windows/Linux compile the shaders as authored (GLSL 4.5). macOS
            // caps at GLSL 4.10, so rewrite the source on the fly there only.
            source = PatchGLSLForGL41(source);

            // GLSL 410 matches varyings between stages by NAME (explicit
            // location-based interface matching for struct varyings is 4.4+).
            // The shaders name the vertex output block "Output" and the fragment
            // input block "Input"; unify them so linking succeeds.
            if (type == GL_FRAGMENT_SHADER) {
                source = std::regex_replace(source, std::regex(R"(\bInput\b)"), "Output");
                // macOS GL 4.1 allows only 16 fragment samplers; the sampler
                // array must match Renderer2D's MaxTextureSlots (16).
                source = std::regex_replace(source, std::regex(R"(u_Textures\s*\[\s*32\s*\])"), "u_Textures[16]");
            }
#endif

            GLuint shader = glCreateShader(type);

            const GLchar* sourceCStr = source.c_str();
            glShaderSource(shader, 1, &sourceCStr, 0);

            glCompileShader(shader);

            GLint isCompiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
            if (isCompiled == GL_FALSE) {
                GLint maxLength = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

                std::vector<GLchar> infoLog(maxLength);
                glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

                glDeleteShader(shader);

                std::string errorMessage = "Shader compilation failure: " + std::to_string(type) + (
                    !infoLog.empty() ? infoLog.data() : "");
                throw ShaderCompilationException(errorMessage);
                break;
            }

            glAttachShader(program, shader);
            glShaderIDs[glShaderIDIndex++] = shader;
        }

        m_Renderer2DID = program;

        // Link our program
        glLinkProgram(program);

        // Note the different functions here: glGetProgram* instead of glGetShader*.
        GLint isLinked = 0;
        glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
        if (isLinked == GL_FALSE) {
            GLint maxLength = 0;
            glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

            // The maxLength includes the NULL character
            std::vector<GLchar> infoLog(maxLength);
            glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

            // We don't need the program anymore.
            glDeleteProgram(program);

            for (auto id : glShaderIDs)
                glDeleteShader(id);

            throw ShaderCompilationException("Shader link failure: " + std::string(infoLog.data()));
            return;
        }

        for (auto id : glShaderIDs) {
            glDetachShader(program, id);
            glDeleteShader(id);
        }
    }

    int Shader::getCurrentActiveProgram() {
        GLint prog = 0;
        glGetIntegerv(GL_CURRENT_PROGRAM, &prog);
        return prog;
    }

    void Shader::bind() const {
        glUseProgram(m_Renderer2DID);
    }

    void Shader::unbind() const {
        glUseProgram(0);
    }

    void Shader::copyCustomUniformsTo(Ref<Shader>& target) const {
        for (auto& [name, value] : this->m_CustomUniforms) {
            auto [type, buffer] = value;
            switch (type) {
            case ShaderDataType::Int: {
                auto data = *(int*)buffer;
                target->setInt(name, data);
                target->saveCustomUniformValue(name, type, data);
                break;
            }
            case ShaderDataType::Float: {
                auto data = *(float*)buffer;
                target->setFloat(name, data);
                target->saveCustomUniformValue(name, type, data);
                break;
            }
            case ShaderDataType::Float2: {
                auto data = *(glm::vec2*)buffer;
                target->setFloat2(name, data);
                target->saveCustomUniformValue(name, type, data);
                break;
            }
            case ShaderDataType::Float3: {
                auto data = *(glm::vec3*)buffer;
                target->setFloat3(name, data);
                target->saveCustomUniformValue(name, type, data);
                break;
            }
            case ShaderDataType::Float4: {
                auto data = *(glm::vec4*)buffer;
                target->setFloat4(name, data);
                target->saveCustomUniformValue(name, type, data);
                break;
            }
            default:
                SHADO_CORE_WARN("Custom uniform type not supported");
                break;
            }
        }
    }

    void Shader::setInt(const std::string& name, int value) {
        GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
        glProgramUniform1i(m_Renderer2DID, location, value);
    }

    void Shader::setIntArray(const std::string& name, int* values, uint32_t count) {
        GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
        glProgramUniform1iv(m_Renderer2DID, location, count, values);
    }

    void Shader::setFloat(const std::string& name, float value) {
        GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
        glProgramUniform1f(m_Renderer2DID, location, value);
    }

    void Shader::setFloat2(const std::string& name, const glm::vec2& value) {
        GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
        glProgramUniform2f(m_Renderer2DID, location, value.x, value.y);
    }

    void Shader::setFloat3(const std::string& name, const glm::vec3& value) {
        GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
        glProgramUniform3f(m_Renderer2DID, location, value.x, value.y, value.z);
    }

    void Shader::setFloat4(const std::string& name, const glm::vec4& value) {
        GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
        glProgramUniform4f(m_Renderer2DID, location, value.x, value.y, value.z, value.w);
    }

    void Shader::setMat3(const std::string& name, const glm::mat3& value) {
        GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
        glProgramUniformMatrix3fv(m_Renderer2DID, location, 1, GL_FALSE, glm::value_ptr(value));
    }

    void Shader::setMat4(const std::string& name, const glm::mat4& value) {
        GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
        glProgramUniformMatrix4fv(m_Renderer2DID, location, 1, GL_FALSE, glm::value_ptr(value));
    }

    std::map<std::string, ShaderDataType> Shader::getActiveUniforms() {
        std::map<std::string, ShaderDataType> uniforms;
        int count = 0;
        glGetProgramiv(m_Renderer2DID, GL_ACTIVE_ATTRIBUTES, &count);

        for (int i = 0; i < count; i++) {
            GLchar name[256]; // name of the uniform
            GLsizei length; // length of the name
            GLint size; // size of the uniform
            GLenum type; // type of the uniform
            glGetActiveUniform(m_Renderer2DID, (GLuint)i, sizeof(name) - 1, &length, &size, &type, name);

            std::string uniformName(name, length);
            uniforms[uniformName] = ShaderDataTypeFromGLType(type);
        }

        return uniforms;
    }

    int Shader::getInt(const std::string& name) {
        int currentProgram = getCurrentActiveProgram();
        this->bind();

        int result;
        glGetUniformiv(m_Renderer2DID, glGetUniformLocation(m_Renderer2DID, name.c_str()), &result);

        // Bind back the previous program
        glUseProgram(currentProgram);
        return result;
    }

    float Shader::getFloat(const std::string& name) {
        int currentProgram = getCurrentActiveProgram();
        this->bind();
        float result;
        glGetUniformfv(m_Renderer2DID, glGetUniformLocation(m_Renderer2DID, name.c_str()), &result);

        // Bind back the previous program
        glUseProgram(currentProgram);
        return result;
    }

    glm::vec2 Shader::getFloat2(const std::string& name) {
        int currentProgram = getCurrentActiveProgram();
        this->bind();
        glm::vec2 result;
#if defined(SHADO_PLATFORM_MACOS)
        // glGetnUniformfv is GL 4.5 (ARB_robustness) and null on macOS 4.1.
        glGetUniformfv(m_Renderer2DID, glGetUniformLocation(m_Renderer2DID, name.c_str()),
                        glm::value_ptr(result));
#else
        glGetnUniformfv(m_Renderer2DID, glGetUniformLocation(m_Renderer2DID, name.c_str()), sizeof(glm::vec2),
                        glm::value_ptr(result));
#endif
        // Bind back the previous program
        glUseProgram(currentProgram);
        return result;
    }

    glm::vec3 Shader::getFloat3(const std::string& name) {
        int currentProgram = getCurrentActiveProgram();
        this->bind();
        glm::vec3 result;
#if defined(SHADO_PLATFORM_MACOS)
        glGetUniformfv(m_Renderer2DID, glGetUniformLocation(m_Renderer2DID, name.c_str()),
                        glm::value_ptr(result));
#else
        glGetnUniformfv(m_Renderer2DID, glGetUniformLocation(m_Renderer2DID, name.c_str()), sizeof(glm::vec3),
                        glm::value_ptr(result));
#endif
        // Bind back the previous program
        glUseProgram(currentProgram);
        return result;
    }

    glm::vec4 Shader::getFloat4(const std::string& name) {
        int currentProgram = getCurrentActiveProgram();
        this->bind();
        glm::vec4 result;
#if defined(SHADO_PLATFORM_MACOS)
        glGetUniformfv(m_Renderer2DID, glGetUniformLocation(m_Renderer2DID, name.c_str()),
                        glm::value_ptr(result));
#else
        glGetnUniformfv(m_Renderer2DID, glGetUniformLocation(m_Renderer2DID, name.c_str()), sizeof(glm::vec4),
                        glm::value_ptr(result));
#endif
        // Bind back the previous program
        glUseProgram(currentProgram);
        return result;
    }
}
