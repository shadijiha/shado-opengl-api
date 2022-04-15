#include "Shader.h"
#include <GL/glew.h>
#include <fstream>
#include <array>
#include "debug/Debug.h"
#include "glm/gtc/type_ptr.hpp"

namespace Shado {

	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;
		return 0;
	}

	Shader::Shader(const std::string& filepath)
	{
		std::string source = readFile(filepath);
		auto shaderSources = preProcess(source);
		compile(shaderSources);

		// Extract name from filepath
		auto lastSlash = filepath.find_last_of("/\\");
		lastSlash = lastSlash == std::string::npos ? 0 : lastSlash + 1;
		auto lastDot = filepath.rfind('.');
		auto count = lastDot == std::string::npos ? filepath.size() - lastSlash : lastDot - lastSlash;
		m_Name = filepath.substr(lastSlash, count);


		// See if the vertex Shader constains the basic uniforms	
		std::vector<std::string> requiredUniforms{ "u_ViewProjection", "u_Transform", "a_Position" };
		for (const std::string& uni : requiredUniforms) {
			if (source.find(uni) == std::string::npos) {
				// SHADO_CORE_WARN("{0} shader does not contain the following uniform/layout: {1}", m_Name,  uni);
				// Logger is not working properly
			}

		}
	}

	Shader::Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{


		std::unordered_map<GLenum, std::string> sources;
		sources[GL_VERTEX_SHADER] = vertexSrc;
		sources[GL_FRAGMENT_SHADER] = fragmentSrc;
		compile(sources);
	}

	Shader::~Shader()
	{


		glDeleteProgram(m_Renderer2DID);
	}

	std::string Shader::readFile(const std::string& filepath)
	{


		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary); // ifstream closes itself due to RAII
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size != -1)
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
			} else
			{
				SHADO_CORE_ERROR("Could not read from file " + filepath);
			}
		} else
		{
			SHADO_CORE_ASSERT(false, "Could not open file " + filepath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> Shader::preProcess(const std::string& source)
	{
		std::unordered_map<GLenum, std::string> shaderSources;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		size_t pos = source.find(typeToken, 0); //Start of shader type declaration line
		while (pos != std::string::npos)
		{
			size_t eol = source.find_first_of("\r\n", pos); //End of shader type declaration line
			SHADO_CORE_ASSERT(eol != std::string::npos, "Syntax error");

			size_t begin = pos + typeTokenLength + 1; //Start of shader type name (after "#type " keyword)
			std::string type = source.substr(begin, eol - begin);
			SHADO_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type specified");

			size_t nextLinePos = source.find_first_not_of("\r\n", eol); //Start of shader code after shader type declaration line
			//HZ_CORE_ASSERT(nextLinePos != std::string::npos, "Syntax error");
			pos = source.find(typeToken, nextLinePos); //Start of next shader type declaration line

			shaderSources[ShaderTypeFromString(type)] = (pos == std::string::npos) ? source.substr(nextLinePos) : source.substr(nextLinePos, pos - nextLinePos);
		}

		return shaderSources;
	}

	void Shader::compile(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();
		SHADO_CORE_ASSERT(shaderSources.size() <= 2, "We only support 2 shaders for now");

		std::array<GLenum, 2> glShaderIDs;
		int glShaderIDIndex = 0;
		for (auto& kv : shaderSources)
		{
			GLenum type = kv.first;
			const std::string& source = kv.second;

			GLuint shader = glCreateShader(type);

			const GLchar* sourceCStr = source.c_str();
			glShaderSource(shader, 1, &sourceCStr, 0);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				glDeleteShader(shader);

				SHADO_CORE_ERROR(infoLog.data());
				SHADO_CORE_ASSERT(false, type + " Shader compilation failure!");
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
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore.
			glDeleteProgram(program);

			for (auto id : glShaderIDs)
				glDeleteShader(id);

			SHADO_CORE_ERROR(infoLog.data());
			//HZ_CORE_ASSERT(false, "Shader link failure!");
			return;
		}

		for (auto id : glShaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}
	}

	void Shader::bind() const
	{
		glUseProgram(m_Renderer2DID);
	}

	void Shader::unbind() const
	{
		glUseProgram(0);
	}

	void Shader::setInt(const std::string& name, int value)
	{


		uploadUniformInt(name, value);
	}

	void Shader::setIntArray(const std::string& name, int* values, uint32_t count)
	{
		uploadUniformIntArray(name, values, count);
	}

	void Shader::setFloat(const std::string& name, float value)
	{


		uploadUniformFloat(name, value);
	}

	void Shader::setFloat3(const std::string& name, const glm::vec3& value)
	{


		uploadUniformFloat3(name, value);
	}

	void Shader::setFloat4(const std::string& name, const glm::vec4& value)
	{


		uploadUniformFloat4(name, value);
	}

	void Shader::setMat4(const std::string& name, const glm::mat4& value)
	{


		uploadUniformMat4(name, value);
	}

	void Shader::uploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
		glUniform1i(location, value);
	}

	void Shader::uploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void Shader::uploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
		glUniform1f(location, value);
	}

	void Shader::uploadUniformFloat2(const std::string& name, const glm::vec2& value)
	{
		GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
		glUniform2f(location, value.x, value.y);
	}

	void Shader::uploadUniformFloat3(const std::string& name, const glm::vec3& value)
	{
		GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
		glUniform3f(location, value.x, value.y, value.z);
	}

	void Shader::uploadUniformFloat4(const std::string& name, const glm::vec4& value)
	{
		GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
		glUniform4f(location, value.x, value.y, value.z, value.w);
	}

	void Shader::uploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void Shader::uploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_Renderer2DID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}
}
