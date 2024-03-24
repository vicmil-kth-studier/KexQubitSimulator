#pragma once
#include "L2_shader.h"

namespace vicmil {
/**
 * Create a vertex array object, there usually have to exist one of those for the program to work!
*/
GLuint create_vertex_array_object() {
    GLuint vao;
    glGenVertexArraysOES(1, &vao);
    glBindVertexArrayOES(vao);
    return vao;
}

class GLBuffer {
public:
    unsigned int buffer_id;
    unsigned int type;
    unsigned int size;
    bool dynamic_buffer;
    GLBuffer() {}
    static GLBuffer generate_buffer(unsigned int size_in_bytes, const void* data, unsigned int buffer_type, bool dynamic_buffer_=false) {
        GLBuffer new_buffer = GLBuffer();
        new_buffer.type = buffer_type;
        new_buffer.size = size_in_bytes;
        new_buffer.dynamic_buffer = dynamic_buffer_;
        GLCall(glGenBuffers(1, &new_buffer.buffer_id)); // Create the buffer
        new_buffer.bind_buffer();
        if(dynamic_buffer_) {
            GLCall(glBufferData(buffer_type, size_in_bytes, data, GL_DYNAMIC_DRAW)); // Set data in buffer
        }
        else {
            GLCall(glBufferData(buffer_type, size_in_bytes, data, GL_STATIC_DRAW)); // Set data in buffer
        }
        return new_buffer;
    }
    void overwrite_buffer_data(unsigned int size_in_bytes, const void* data) {
        bind_buffer();
        size = size_in_bytes;
        if(dynamic_buffer) {
            GLCall(glBufferData(type, size_in_bytes, data, GL_DYNAMIC_DRAW)); // Set data in buffer
        }
        else {
            GLCall(glBufferData(type, size_in_bytes, data, GL_STATIC_DRAW)); // Set data in buffer
        }
    }
    void substitute_buffer_data(unsigned int size_in_bytes, const void* data, unsigned int offset=0) {
        GLCall(glBufferSubData(type, offset, size_in_bytes, data));
    }
    void bind_buffer() {
        // You must first bind a buffer in order to use or modify it, only one buffer can be binded at the same time
        glBindBuffer(type, buffer_id);
    }
    ~GLBuffer() {
        //glDeleteBuffers(1, &buffer_id);
    }
    unsigned int number_of_floats() {
        return size / sizeof(float);
    }
    unsigned int number_of_unsigned_ints() {
        return size / sizeof(unsigned int);
    }
};

class GLIndexBuffer {
public:
    GLBuffer buffer;
    GLIndexBuffer() {}
    static GLIndexBuffer create_buffer(std::vector<unsigned int> data) {
        GLIndexBuffer new_buffer = GLIndexBuffer();
        new_buffer.buffer = GLBuffer::generate_buffer(sizeof(unsigned int) * data.size(), &data[0], GL_ELEMENT_ARRAY_BUFFER);
        return new_buffer;
    }
};

class GLVertexBuffer {
public:
    GLBuffer buffer;
    GLVertexBuffer() {}
    static GLVertexBuffer create_buffer(std::vector<float> data) {
        GLVertexBuffer new_buffer = GLVertexBuffer();
        new_buffer.buffer = GLBuffer::generate_buffer(sizeof(float) * data.size(), &data[0], GL_ARRAY_BUFFER);
        return new_buffer;
    }
};

class UniformBuffer {
public:
    static void set_4f(std::vector<float> data, GPUProgram& program, std::string shader_variable_name) {
        if (data.size() != 4) {
            std::cout << "Wrong uniform buffer size" << std::endl;
            throw;
        }
        int location = glGetUniformLocation(program.id, shader_variable_name.c_str());
        if (location == -1) {
            std::cout << "Uniform not found" << std::endl;
            throw;
        }
        glUniform4f(location, data[0], data[1], data[2], data[3]);
    }
    /**
     * Set the data in a uniform buffer to a 4x4 matrix
     * 
     * @arg data: The data you want to put in the buffer, 4x4 matrix
     * @arg program: The GPU program, it contains information about the uniform variable
     * @arg shader_variable_name: the name of the uniform buffer variable in the program
    */
    static void set_mat4f(glm::mat4 data, GPUProgram& program, std::string shader_variable_name) {
        int location = glGetUniformLocation(program.id, shader_variable_name.c_str());
        if (location == -1) {
            std::cout << "Uniform not found: " << shader_variable_name << std::endl;
            return;
        }
        const unsigned int matrix_count = 1;
        const char transpose_matrix = GL_FALSE;
        GLCall(glUniformMatrix4fv(location, matrix_count, transpose_matrix, &data[0][0]));
    }
};

class VertexBufferElement {
public:
    unsigned int type;
    unsigned int count;
    unsigned char normalized = GL_FALSE;
    VertexBufferElement() {}
    static VertexBufferElement create_element(unsigned int type_, unsigned int count_, unsigned char normalized_ = GL_FALSE) {
        VertexBufferElement new_element = VertexBufferElement();
        new_element.type = type_;
        new_element.count = count_;
        new_element.normalized = normalized_;
        return new_element;
    }

    unsigned int get_size() {
        if(type == GL_FLOAT) {
            return sizeof(float) * count;
        }
        throw "Unknown type";
    }
};

/**
 * Help class specify how the vertex data are aligned in the memory
*/
class VertexBufferLayout {
public:
    std::vector<VertexBufferElement> buffer_elements;
    VertexBufferLayout() {}
    static VertexBufferLayout from_elements(std::vector<VertexBufferElement> buffer_elements_) {
        VertexBufferLayout new_layout = VertexBufferLayout();
        new_layout.buffer_elements = buffer_elements_;
        return new_layout;
    }
    unsigned int get_size() {
        int size_ = 0;
        for(unsigned int i = 0; i < buffer_elements.size(); i++) {
            size_ += buffer_elements[i].get_size();
        }
        return size_;
    }
    void set_vertex_buffer_layout() {
        unsigned int offset = 0; // Offset is between each element
        unsigned int stride = this->get_size(); // Stride is between a set of all elements
        for(unsigned int i = 0; i < buffer_elements.size(); i++) {
            GLCall(glEnableVertexAttribArray(i));
            GLCall(glVertexAttribPointer(
                i, 
                buffer_elements[i].count, 
                buffer_elements[i].type, 
                buffer_elements[i].normalized,
                stride,
                reinterpret_cast<const void*>(offset)
            ));
            offset += buffer_elements[i].get_size();
        }
    }
};

/**
 * It is often good to group an index buffer and a vertex buffer as a pair
 *  - A vertex buffer contains all the triangle corners in a jumbled mess
 *  - The Index buffer chooses aming all the corners to decide which corners should make up a triangle
*/
class IndexVertexBufferPair {
public:
    GLBuffer vertex_buffer;
    GLBuffer index_buffer;
    static IndexVertexBufferPair from_raw_data(void* index_data, unsigned int index_data_byte_size, void* vertex_data, unsigned int vertex_data_byte_size) {
        IndexVertexBufferPair new_buffer_pair;
        new_buffer_pair.vertex_buffer = GLBuffer::generate_buffer(vertex_data_byte_size, vertex_data, GL_ARRAY_BUFFER);
        new_buffer_pair.index_buffer = GLBuffer::generate_buffer(index_data_byte_size, index_data, GL_ELEMENT_ARRAY_BUFFER);
        return new_buffer_pair;
    }
    /**
     * Overwrite all the data in the vertex and index buffer
    */
    void overwrite_data(void* index_data, unsigned int index_data_byte_size, void* vertex_data, unsigned int vertex_data_byte_size) {
        vertex_buffer.overwrite_buffer_data(vertex_data_byte_size, vertex_data);
        index_buffer.overwrite_buffer_data(index_data_byte_size, index_data);
    }
    // Set the buffer layout to follow the shader example specified in L2_shader.h
    static void set_vertex_buffer_layout_to_example() {
        std::vector<VertexBufferElement> elements = {
            VertexBufferElement::create_element(GL_FLOAT, 3),
            VertexBufferElement::create_element(GL_FLOAT, 4),
            VertexBufferElement::create_element(GL_FLOAT, 2)
        };
        VertexBufferLayout layout = VertexBufferLayout::from_elements(elements);
        layout.set_vertex_buffer_layout();
    }
    /**
     * Bind the vertex and index buffers on the GPU. This means that it 
     *  is these buffers the GPU refers to until another buffer calls bind()
     *  there can only exist one bound buffer of each type at one time
    */
    void bind() {
        vertex_buffer.bind_buffer();
        index_buffer.bind_buffer();
    }
    /**
     * Use buffer to perform program on GPU, the GPU Program must be bound before this call
     * @arg triangle_count: The number of triangles to use, if it is -1, use all triangles in index buffer
     * @arg offset_in_bytes: The offset of where to begin in index buffer, great if you only want to use
     *    a part of the buffer, otherwise just leave it
    */
    void draw(int triangle_count=-1, unsigned int offset_in_bytes=0) {
        if(triangle_count > 0) {
            GLCall(glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_INT, reinterpret_cast<const void*>(offset_in_bytes)));
        }
        else {
            Debug("index buffer size: " << index_buffer.number_of_unsigned_ints());
            GLCall(glDrawElements(GL_TRIANGLES, index_buffer.number_of_unsigned_ints(), GL_UNSIGNED_INT, nullptr));
        }
    }
};
}