#include "Buffer.h"

#include "GL/glew.h"

namespace Shado {
    ShaderDataType ShaderDataTypeFromGLType(uint32_t openGLType) {
        switch (openGLType) {
        case GL_FLOAT: return ShaderDataType::Float;
        case GL_FLOAT_VEC2: return ShaderDataType::Float2;
        case GL_FLOAT_VEC3: return ShaderDataType::Float3;
        case GL_FLOAT_VEC4: return ShaderDataType::Float4;
        case GL_INT: return ShaderDataType::Int;
        case GL_INT_VEC2: return ShaderDataType::Int2;
        case GL_INT_VEC3: return ShaderDataType::Int3;
        case GL_INT_VEC4: return ShaderDataType::Int4;
        case GL_BOOL: return ShaderDataType::Bool;
        case GL_FLOAT_MAT3: return ShaderDataType::Mat3;
        case GL_FLOAT_MAT4: return ShaderDataType::Mat4;
        case GL_SAMPLER_2D: return ShaderDataType::Sampler2D;
        }

        SHADO_CORE_ASSERT(false, "Unknown OpenGL type!");
        return ShaderDataType::None;
    }

    VertexBuffer::VertexBuffer(uint32_t size) {
        glCreateBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
    }

    VertexBuffer::VertexBuffer(float* vertices, uint32_t size) {
        glCreateBuffers(1, &m_RendererID);
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, size, vertices, GL_STATIC_DRAW);
    }

    VertexBuffer::~VertexBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    void VertexBuffer::bind() const {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
    }

    void VertexBuffer::unBind() const {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    Ref<VertexBuffer> VertexBuffer::create(float* vertices, uint32_t size) {
        return CreateRef<VertexBuffer>(vertices, size);
    }

    void VertexBuffer::setData(const void* data, size_t size) {
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferSubData(GL_ARRAY_BUFFER, 0, size, data);
    }

    Ref<VertexBuffer> VertexBuffer::create(uint32_t size) {
        return CreateRef<VertexBuffer>(size);
    }

    // ========================================
    Ref<IndexBuffer> IndexBuffer::create(uint32_t* indices, uint32_t count) {
        return CreateRef<IndexBuffer>(indices, count);
    }

    IndexBuffer::IndexBuffer(uint32_t* indices, uint32_t count)
        : m_Count(count) {
        glCreateBuffers(1, &m_RendererID);

        // GL_ELEMENT_ARRAY_BUFFER is not valid without an actively bound VAO
        // Binding with GL_ARRAY_BUFFER allows the data to be loaded regardless of VAO state. 
        glBindBuffer(GL_ARRAY_BUFFER, m_RendererID);
        glBufferData(GL_ARRAY_BUFFER, count * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    IndexBuffer::~IndexBuffer() {
        glDeleteBuffers(1, &m_RendererID);
    }

    void IndexBuffer::bind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_RendererID);
    }

    void IndexBuffer::unBind() const {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}
