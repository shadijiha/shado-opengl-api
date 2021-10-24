#pragma once
#include <string>
#include <unordered_map>
#include "glm/glm.hpp"

namespace Shado {
	class Shader
	{
	public:
		Shader(const std::string& filepath);
		Shader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~Shader();

		void bind() const;
		void unbind() const;

		void setInt(const std::string& name, int value);
		void setIntArray(const std::string& name, int* values, uint32_t count) ;
		void setFloat(const std::string& name, float value) ;
		void setFloat3(const std::string& name, const glm::vec3& value) ;
		void setFloat4(const std::string& name, const glm::vec4& value) ;
		void setMat4(const std::string& name, const glm::mat4& value);

		const std::string& getName() const { return m_Name; }

		void uploadUniformInt(const std::string& name, int value);
		void uploadUniformIntArray(const std::string& name, int* values, uint32_t count);

		void uploadUniformFloat(const std::string& name, float value);
		void uploadUniformFloat2(const std::string& name, const glm::vec2& value);
		void uploadUniformFloat3(const std::string& name, const glm::vec3& value);
		void uploadUniformFloat4(const std::string& name, const glm::vec4& value);

		void uploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void uploadUniformMat4(const std::string& name, const glm::mat4& matrix);
	private:
		std::string readFile(const std::string& filepath);
		std::unordered_map<unsigned int, std::string> preProcess(const std::string& source);
		void compile(const std::unordered_map<unsigned int, std::string>& shaderSources);
	private:
		uint32_t m_Renderer2DID;
		std::string m_Name;
	};
}
