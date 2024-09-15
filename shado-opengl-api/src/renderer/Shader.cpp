#include "Shader.h"
#include <GL/glew.h>
#include <fstream>
#include <array>

#include "debug/Debug.h"
#include "glm/gtc/type_ptr.hpp"

namespace Shado {
    static GLenum ShaderTypeFromString(const std::string& type) {
        if (type == "vertex")
            return GL_VERTEX_SHADER;
        if (type == "fragment" || type == "pixel")
            return GL_FRAGMENT_SHADER;
        return 0;
    }

    Shader::Shader(const std::filesystem::path& path)
        : filepath(path) {
        std::string source = readFile(path);
        auto shaderSources = preProcess(source);
        compile(shaderSources);

        // Extract name from filepath
        std::string filepath = path.string();
        auto lastSlash = filepath.find_last_of("/\\");
        lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
        auto lastDot = filepath.rfind('.');
        auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
        m_Name = filepath.substr(lastSlash, count);


        // See if the vertex Shader constains the basic uniforms	
        std::vector<std::string> requiredUniforms{"u_ViewProjection", "u_Transform", "a_Position"};
        for (const std::string& uni : requiredUniforms) {
            if (source.find(uni) == std::string::npos) {
                // SHADO_CORE_WARN("{0} shader does not contain the following uniform/layout: {1}", m_Name,  uni);
                // Logger is not working properly
            }
        }
    }

    Shader::Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
        : m_Name(name) {
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
            const std::string& source = kv.second;

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

                std::string errorMessage = "Shader compilation failure: " + std::to_string(type) + infoLog.data();
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
        glGetnUniformfv(m_Renderer2DID, glGetUniformLocation(m_Renderer2DID, name.c_str()), sizeof(glm::vec2),
                        glm::value_ptr(result));
        // Bind back the previous program
        glUseProgram(currentProgram);
        return result;
    }

    glm::vec3 Shader::getFloat3(const std::string& name) {
        int currentProgram = getCurrentActiveProgram();
        this->bind();
        glm::vec3 result;
        glGetnUniformfv(m_Renderer2DID, glGetUniformLocation(m_Renderer2DID, name.c_str()), sizeof(glm::vec3),
                        glm::value_ptr(result));
        // Bind back the previous program
        glUseProgram(currentProgram);
        return result;
    }

    glm::vec4 Shader::getFloat4(const std::string& name) {
        int currentProgram = getCurrentActiveProgram();
        this->bind();
        glm::vec4 result;
        glGetnUniformfv(m_Renderer2DID, glGetUniformLocation(m_Renderer2DID, name.c_str()), sizeof(glm::vec4),
                        glm::value_ptr(result));
        // Bind back the previous program
        glUseProgram(currentProgram);
        return result;
    }
}
