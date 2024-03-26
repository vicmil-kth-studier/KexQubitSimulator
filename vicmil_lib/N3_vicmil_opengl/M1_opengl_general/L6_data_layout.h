#include "L5_gpu_setup.h"

namespace vicmil {

enum DataType {
    FLOAT32,
    DOUBLE64,
    INT8,
    INT16,
    INT32,
    INT64
};

struct ColorF3 {
    float r;
    float g;
    float b;
    ColorF3() {}
    ColorF3(glm::dvec3 color) {
        r = color.r;
        g = color.b;
        b = color.b;
    }
    static ColorF3 from_str(std::string r_, std::string g_, std::string b_) {
        return ColorF3(glm::dvec3(std::stof(r_), std::stof(g_), std::stof(b_)));
    }
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return {VertexBufferElement::create_element(GL_FLOAT, 3)};
    }
};

struct ColorF4 {
    float r;
    float g;
    float b;
    float a;
    ColorF4() {}
    ColorF4(glm::dvec4 color) {
        r = color.r;
        g = color.b;
        b = color.b;
        a = color.a;
    }
    static ColorF4 from_str(std::string r_, std::string g_, std::string b_, std::string a_) {
        return ColorF4(glm::dvec4(std::stof(r_), std::stof(g_), std::stof(b_), std::stof(a_)));
    }
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return {VertexBufferElement::create_element(GL_FLOAT, 4)};
    }
};

struct TextureCoordF2 {
    float x;
    float y;
    TextureCoordF2() {}
    TextureCoordF2(glm::dvec2 pos) {
        x = pos.x;
        y = pos.y;
    }
    static TextureCoordF2 from_str(std::string x_, std::string y_) {
        return TextureCoordF2(glm::dvec2(std::stof(x_), std::stof(y_)));
    }
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return {VertexBufferElement::create_element(GL_FLOAT, 2)};
    }
};

struct PositionF3 {
    float x;
    float y;
    float z;
    PositionF3() {}
    PositionF3(glm::dvec3 position) {
        x = position.x;
        y = position.y;
        z = position.z;
    }
    static PositionF3 from_str(std::string x_, std::string y_, std::string z_) {
        return PositionF3(glm::dvec3(std::stof(x_), std::stof(y_), std::stof(z_)));
    }
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return {VertexBufferElement::create_element(GL_FLOAT, 3)};
    }
};

struct PositionF2 {
    float x;
    float y;
    PositionF2() {}
    PositionF2(glm::dvec2 position) {
        x = position.x;
        y = position.y;
    }
    static PositionF2 from_str(std::string x_, std::string y_) {
        return PositionF2(glm::dvec2(std::stof(x_), std::stof(y_)));
    }
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return {VertexBufferElement::create_element(GL_FLOAT, 2)};
    }
};

struct NormalF3 {
    float x;
    float y;
    float z;
    NormalF3() {}
    NormalF3(glm::dvec3 normal) {
        x = normal.x;
        y = normal.y;
        z = normal.z;
    }
    static NormalF3 from_str(std::string x_, std::string y_, std::string z_) {
        return NormalF3(glm::dvec3(std::stof(x_), std::stof(y_), std::stof(z_)));
    }
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return {VertexBufferElement::create_element(GL_FLOAT, 3)};
    }
};

/**
 * This is a layout that can describe many things, such as models, images, text, colored rectangles etc, etc
 * It is a fit all solution for many problems
 * 
 * Pros:
 *  - Makes it possible to merge a lot of different draw operations into one, which can greatly improve performance
 * 
 * Cons:
 *  - may take more memory than using a specialized shader
*/
struct ColorF3PositionF3TextureCoordF2 {
    ColorF3 color;
    PositionF3 position;
    TextureCoordF2 texture_coordinate;
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        std::vector<VertexBufferElement> vec;
        vicmil::vec_extend(vec, ColorF3::get_vertex_buffer_layout());
        vicmil::vec_extend(vec, PositionF3::get_vertex_buffer_layout());
        vicmil::vec_extend(vec, TextureCoordF2::get_vertex_buffer_layout());
        return vec;
    }
};
}
