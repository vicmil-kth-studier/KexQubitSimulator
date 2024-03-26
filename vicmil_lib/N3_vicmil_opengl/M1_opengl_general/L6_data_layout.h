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
        g = color.g;
        b = color.b;
    }
    ColorF3(float r_, float g_, float b_) {
        r = r_;
        g = g_;
        b = b_;
    }
    static ColorF3 from_str(std::string r_, std::string g_, std::string b_) {
        return ColorF3(glm::dvec3(std::stof(r_), std::stof(g_), std::stof(b_)));
    }
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return {VertexBufferElement::create_element(GL_FLOAT, 3)};
    }
    std::string to_string() {
        return "r: " + std::to_string(r) + ", g: " + std::to_string(g)  + ", b: " + std::to_string(b);
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
    TextureCoordF2(float tex_x, float tex_y) {
        x = tex_x;
        y = tex_y;
    }
    static TextureCoordF2 from_str(std::string x_, std::string y_) {
        return TextureCoordF2(glm::dvec2(std::stof(x_), std::stof(y_)));
    }
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return {VertexBufferElement::create_element(GL_FLOAT, 2)};
    }
    std::string to_string() {
        return "x: " + std::to_string(x) + ", y: " + std::to_string(y);
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
    PositionF3(float x_, float y_, float z_) {
        x = x_;
        y = y_;
        z = z_;
    }
    static PositionF3 from_str(std::string x_, std::string y_, std::string z_) {
        return PositionF3(glm::dvec3(std::stof(x_), std::stof(y_), std::stof(z_)));
    }
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return {VertexBufferElement::create_element(GL_FLOAT, 3)};
    }
    std::string to_string() {
        return "x: " + std::to_string(x) + ", y: " + std::to_string(y)  + ", z: " + std::to_string(z);
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
struct PositionF3TextureCoordF2ColorF3 {
    PositionF3 position;
    TextureCoordF2 texture_coordinate;
    ColorF3 color;
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        std::vector<VertexBufferElement> vec;
        vicmil::vec_extend(vec, PositionF3::get_vertex_buffer_layout());
        vicmil::vec_extend(vec, TextureCoordF2::get_vertex_buffer_layout());
        vicmil::vec_extend(vec, ColorF3::get_vertex_buffer_layout());
        return vec;
    }
    PositionF3TextureCoordF2ColorF3() {}
    PositionF3TextureCoordF2ColorF3(float x, float y, float z, float tex_x, float tex_y, float r, float g, float b) {
        position = PositionF3(x, y, z);
        texture_coordinate = TextureCoordF2(tex_x, tex_y);
        color = ColorF3(r, g, b);
    }
    std::string to_string() {
        return position.to_string() + ":   " + texture_coordinate.to_string() + ":   " + color.to_string();

    }
};
}
