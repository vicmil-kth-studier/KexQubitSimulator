#include "L2_view_transform.h"

namespace vicmil {
namespace general_gpu_setup {
/**
 * This is an example of a vertex-fragment shader pair that can be used for many things, 
 *   more specialized shaders can use less memory, be faster, or do more advanced things
 *   but this is a one size fits all solution that can do most basic things in both 2D and 3D such as
 *   - Drawing textures
 *   - Drawing triangles
 *   - Drawing text
 *   - Drawing models
 *   - Handle camera perspecitives and rotations
 * 
 *  NOTE: No fancy shading or lighting
 *  NOTE: You need to configure the GPU correctly for everything to work! (see examples)
*/
const std::string vert_shader =
vicmil::SHADER_VERSON_OPENGL_ES +
    "uniform mat4 u_MVP;\n"
    "attribute vec3 position;    \n"
    "attribute vec2 tex_coord;    \n"
    "attribute vec3 color_;    \n"
    "varying vec2 v_tex_coord;\n"   
    "varying vec4 v_Color;\n"
    "void main()                  \n"
    "{                            \n"
    "   v_Color = vec4(color_.x, color_.y, color_.z, 1.0);\n"
    "   v_tex_coord = tex_coord;\n"
    "   gl_Position = u_MVP * vec4(position.xyz, 1.0);  \n"
    "}                            \n";

const std::string frag_shader =
vicmil::SHADER_VERSON_OPENGL_ES + 
    "precision mediump float;\n"
    "varying vec2 v_tex_coord;\n"
    "varying vec4 v_Color;\n"
    "uniform sampler2D our_texture;\n"
    "void main()                                  \n"
    "{                                            \n"
    "  gl_FragColor = v_Color;\n"
    "  if(v_tex_coord.x > 0) {\n"
    "     gl_FragColor = texture2D(our_texture, v_tex_coord); \n"
    "     gl_FragColor = vec4(gl_FragColor.x * v_Color.x, gl_FragColor.y * v_Color.y, gl_FragColor.z * v_Color.z, gl_FragColor.w * v_Color.w); \n"
    "  } \n"
    "}                                            \n";

struct Triangle {
    // The three corners of the traingle
    PositionF3TextureCoordF2ColorF3 corner1 = vicmil::PositionF3TextureCoordF2ColorF3(0.0f, 0.5f, 0.0f,   -0.5, 0.0,   1.0, 0.0, 0.0);
    PositionF3TextureCoordF2ColorF3 corner2 = vicmil::PositionF3TextureCoordF2ColorF3(0.5f, -0.5f, 0.0f,   -1.0, 1.0,  0.0, 1.0, 0.0);
    PositionF3TextureCoordF2ColorF3 corner3 = vicmil::PositionF3TextureCoordF2ColorF3(-0.5f, -0.5, 0.0f,   -1.0, 1.0,  0.0, 0.0, 1.0);

    void set_triangle_color(glm::vec4 color) {
        corner1.color = ColorF3(color);
        corner2.color = ColorF3(color);
        corner3.color = ColorF3(color);

        // Disable texture
        corner1.texture_coordinate.x = -1.0;
        corner2.texture_coordinate.x = -1.0;
        corner3.texture_coordinate.x = -1.0;
    }
    void set_triangle_3D_position(glm::vec3 p1, glm::dvec3 p2, glm::dvec3 p3) {
        corner1.position = PositionF3(p1);
        corner2.position = PositionF3(p2);
        corner3.position = PositionF3(p3);
    }
    void set_triangle_2D_position(glm::vec2 p1, glm::dvec2 p2, glm::dvec2 p3) {
        corner1.position = PositionF3(p1.x, p1.y, 0.0);
        corner2.position = PositionF3(p2.x, p2.y, 0.0);
        corner3.position = PositionF3(p3.x, p3.y, 0.0);
    }
    void set_triangle_2D_position(float x1, float y1, float x2, float y2, float x3, float y3) {
        // Set the position and leave z axis to 0, since it is 2D
        corner1.position = PositionF3(x1, y1, 0.0);
        corner2.position = PositionF3(x2, y2, 0.0);
        corner3.position = PositionF3(x3, y3, 0.0);
    }
    void set_texture_position(glm::vec2 tex_coord1, glm::dvec2 tex_coord2, glm::dvec2 tex_coord3, glm::dvec3 color_mask = glm::dvec3(1.0, 1.0, 1.0)) {
        corner1.texture_coordinate = TextureCoordF2(tex_coord1);
        corner2.texture_coordinate = TextureCoordF2(tex_coord2);
        corner3.texture_coordinate = TextureCoordF2(tex_coord3);

        // Reset color
        corner1.color = color_mask;
        corner2.color = color_mask;
        corner3.color = color_mask;
    }
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return PositionF3TextureCoordF2ColorF3::get_vertex_buffer_layout();
    }
    std::string to_string() {
        std::string triangle_str;
        triangle_str += corner1.to_string() + "\n";
        triangle_str += corner2.to_string() + "\n";
        triangle_str += corner3.to_string() + "\n";
        return triangle_str;
    }
};

/**
 * Create 2 triangles to reprent a rectangle of a specified color
*/
static std::vector<Triangle> triangles_from_2d_color_rect(Rect rect, glm::vec4 color) {
    glm::vec3 p1 = glm::vec3(rect.min_x(), rect.min_y(), 0.0); // (0, 0)
    glm::vec3 p2 = glm::vec3(rect.min_x(), rect.max_y(), 0.0); // (0, 1)
    glm::vec3 p3 = glm::vec3(rect.max_x(), rect.min_y(), 0.0); // (1, 0)
    glm::vec3 p4 = glm::vec3(rect.max_x(), rect.max_y(), 0.0); // (1, 1)

    Triangle triangle1 = Triangle();
    triangle1.set_triangle_color(color);
    triangle1.set_triangle_3D_position(p1, p2, p4);

    Triangle triangle2 = Triangle();
    triangle2.set_triangle_color(color);
    triangle2.set_triangle_3D_position(p1, p3, p4);

    std::vector<Triangle> triangles = {triangle1, triangle2};
    return triangles;
}

/**
 * Create 2 triangles to reprent a rectangle of a specified color
*/
static std::vector<Triangle> triangles_from_2d_texture_rect(Rect rect, Rect tex_rect) {
    glm::vec3 p1 = glm::vec3(rect.min_x(), rect.min_y(), 0.0); // (0, 0)
    glm::vec3 p2 = glm::vec3(rect.min_x(), rect.max_y(), 0.0); // (0, 1)
    glm::vec3 p3 = glm::vec3(rect.max_x(), rect.min_y(), 0.0); // (1, 0)
    glm::vec3 p4 = glm::vec3(rect.max_x(), rect.max_y(), 0.0); // (1, 1)

    glm::vec2 tex_p1 = glm::vec2(tex_rect.min_x(), tex_rect.min_y()); // (0, 0)
    glm::vec2 tex_p2 = glm::vec2(tex_rect.min_x(), tex_rect.max_y()); // (0, 1)
    glm::vec2 tex_p3 = glm::vec2(tex_rect.max_x(), tex_rect.min_y()); // (1, 0)
    glm::vec2 tex_p4 = glm::vec2(tex_rect.max_x(), tex_rect.max_y()); // (1, 1)

    Triangle triangle1 = Triangle();
    triangle1.set_texture_position(tex_p1, tex_p2, tex_p4);
    triangle1.set_triangle_3D_position(p1, p2, p4);

    Triangle triangle2 = Triangle();
    triangle2.set_texture_position(tex_p1, tex_p3, tex_p4);
    triangle2.set_triangle_3D_position(p1, p3, p4);

    std::vector<Triangle> triangles = {triangle1, triangle2};
    return triangles;
}

void overwrite_triangles_in_buffer(IndexVertexBufferPair& buffer_pair, std::vector<Triangle> triangles) {
    std::vector<IndexedTriangleI3> index_buffer_triangles = IndexedTriangleI3::linear_indexed_triangles(triangles.size());
    buffer_pair.overwrite_data(
        (void*)(&index_buffer_triangles[0]), 
        index_buffer_triangles.size() * sizeof(IndexedTriangleI3), 
        (void*)(&triangles[0]), 
        triangles.size() * sizeof(Triangle)
    );
}
IndexVertexBufferPair create_triangles_buffer(std::vector<Triangle> triangles) {
    std::vector<IndexedTriangleI3> index_buffer_triangles = IndexedTriangleI3::linear_indexed_triangles(triangles.size());
    IndexVertexBufferPair new_buffer_pair = IndexVertexBufferPair::from_raw_data(
        (void*)(&index_buffer_triangles[0]), 
        index_buffer_triangles.size() * sizeof(IndexedTriangleI3), 
        (void*)(&triangles[0]), 
        triangles.size() * sizeof(Triangle)
    );

    return new_buffer_pair;
}

GPUSetup create_gpu_setup(int width = 1024, int height = 1024) {
    GPUSetup gpu_setup = GPUSetup();

    gpu_setup.window_renderer = vicmil::WindowRendererPair(width, height);
    vicmil::create_vertex_array_object();

    vicmil::general_gpu_setup::Triangle triangle;
    vicmil::IndexedTriangleI3 traingle_index = vicmil::IndexedTriangleI3(0, 1, 2);

    gpu_setup.index_vertex_buffer = vicmil::IndexVertexBufferPair::from_raw_data(
        &traingle_index,
        1 * sizeof(vicmil::IndexedTriangleI3),
        &triangle,
        1 * sizeof(vicmil::general_gpu_setup::Triangle)
    );

    std::vector<vicmil::VertexBufferElement> layout = vicmil::PositionF3TextureCoordF2ColorF3::get_vertex_buffer_layout();
    gpu_setup.vertex_buffer_layout = vicmil::VertexBufferLayout(layout);
    gpu_setup.vertex_buffer_layout.set_vertex_buffer_layout();

    gpu_setup.program.program = vicmil::GPUProgram::from_strings(vicmil::general_gpu_setup::vert_shader, vicmil::general_gpu_setup::frag_shader);
    gpu_setup.program.program.bind_program();

    vicmil::RawImageRGBA image = vicmil::RawImageRGBA();
    image.width = 2;
    image.height = 2;
    image.pixels.resize(2 * 2);
    image.pixels[0] = vicmil::PixelRGBA(0, 0, 255, 255);
    image.pixels[1] = vicmil::PixelRGBA(0, 255, 255, 255);
    image.pixels[2] = vicmil::PixelRGBA(255, 0, 255, 255);
    image.pixels[3] = vicmil::PixelRGBA(255, 255, 255, 255);

    gpu_setup.texture = vicmil::GPUTexture::from_raw_image_rgba(image);
    gpu_setup.texture.bind();

    return gpu_setup;
}
void set_triangles(GPUSetup& gpu_setup, std::vector<Triangle> triangles) {
    overwrite_triangles_in_buffer(gpu_setup.index_vertex_buffer, triangles);
}
void set_view_transform(GPUSetup& gpu_setup, CameraViewTransform transform) {
    gpu_setup.program.uniform_mat4f_buffer_vars["u_MVP"] = transform.mat;
    gpu_setup.program.setup_uniform_buffer_variables();
}
void set_image(GPUSetup& gpu_setup, RawImageRGBA& image) {
    ThrowNotImplemented();
}
}
}
