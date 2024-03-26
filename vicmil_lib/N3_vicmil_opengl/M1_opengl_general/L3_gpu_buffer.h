#pragma once
#include "L2_gpu_program.h"

namespace vicmil {
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
    VertexBufferLayout(std::vector<VertexBufferElement> buffer_elements_) {
        buffer_elements = buffer_elements_;
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
        DisableLogging;
        if(triangle_count > 0) {
            GLCall(glDrawElements(GL_TRIANGLES, triangle_count * 3, GL_UNSIGNED_INT, reinterpret_cast<const void*>(offset_in_bytes)));
        }
        else {
            Debug("index buffer size: " << index_buffer.number_of_unsigned_ints());
            GLCall(glDrawElements(GL_TRIANGLES, index_buffer.number_of_unsigned_ints(), GL_UNSIGNED_INT, nullptr));
        }
    }
};

/**
 * The index buffer is made up of a list of triangles, this can be one of them
 *   The indicies refer to the vertex buffer
*/
struct IndexedTriangleI3 {
    unsigned int index[3]; // index of triangle corner 1, 2 and 3
    IndexedTriangleI3() {}
    IndexedTriangleI3(unsigned int i1, unsigned int i2, unsigned int i3) {
        index[0] = i1;
        index[1] = i2;
        index[2] = i3;
    }
    static IndexedTriangleI3 from_str(std::string i1, std::string i2, std::string i3) {
        return IndexedTriangleI3(std::stof(i1), std::stof(i2), std::stof(i3));
    }
};


}