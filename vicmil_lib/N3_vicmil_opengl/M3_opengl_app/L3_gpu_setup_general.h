#include "L2_view_transform.h"

namespace vicmil {
namespace gpu_setup_general {
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
"attribute vec3 position;\n"  // The position of the vertex(triangle corner)
"attribute vec4 color;\n"     // The color of the vertex(triangle corner)
"attribute vec2 aTexCoord;\n" // The coordinate on the texture the vertex maps to
                              // (if x corordinate is negative I treat it as it doesn't map to any texture)
"\n"
"varying vec4 v_Color;\n"    
"varying vec2 TexCoord;\n"   
"\n"
"void main() {\n"            
"    gl_Position = vec4(position.x, position.y, position.z, 1.0);\n"
"    v_Color = vec4(color.x, color.y, color.z, 1.0);\n"
"    TexCoord = aTexCoord;\n"
"}\n";

const std::string frag_shader =
vicmil::SHADER_VERSON_OPENGL_ES + 
"precision mediump float;\n"
"varying vec4 v_Color;\n"
"varying vec2 TexCoord;\n"
"\n"
"uniform sampler2D ourTexture;\n"
"\n"
"void main() {\n"
"    gl_FragColor = v_Color;\n"
"}\n";

vicmil::GPUProgram create_gpu_program() {
    return vicmil::GPUProgram::from_strings(vert_shader, frag_shader);
}

struct Triangle {
    // The three corners of the traingle
    ColorF3PositionF3TextureCoordF2 corners[3];
    void set_triangle_color(glm::vec4 color) {
        for(int i = 0; i < 3; i++) {
            corners[i].color = ColorF3(color);
        }
    }
    void set_triangle_position(glm::vec3 p1, glm::dvec3 p2, glm::dvec3 p3) {
        corners[0].position = PositionF3(p1);
        corners[1].position = PositionF3(p2);
        corners[2].position = PositionF3(p3);
    }
    // Set all the texture coordinates to -1
    void disable_texture_coord() { 
        for(int i = 0; i < 3; i++) {
            corners[i].texture_coordinate.x = -1.0;
            corners[i].texture_coordinate.y = -1.0;
        }
    }
    void set_texture_cord(glm::vec2 p1, glm::dvec2 p2, glm::dvec2 p3) {
        corners[0].texture_coordinate = TextureCoordF2(p1);
        corners[1].texture_coordinate = TextureCoordF2(p2);
        corners[2].texture_coordinate = TextureCoordF2(p3);
    }
    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return ColorF3PositionF3TextureCoordF2::get_vertex_buffer_layout();
    }
};

// Let everything be made of triangles!
struct Triangles {
    std::vector<Triangle> triangles = {};
    Triangles() {}
    Triangles(const std::vector<Triangle>& triangles_) {
        triangles = triangles_;
    }
    inline static Triangles merge(const std::vector<Triangles>& triangles_list) {
        Triangles new_triangles = Triangles();
        for(int i = 1; i < triangles_list.size(); i++) {
            new_triangles.triangles.insert(new_triangles.triangles.end(), triangles_list[i].triangles.begin(), triangles_list[i].triangles.end());
        }
        return new_triangles;
    }

    /**
     * Get the index triangles of the current triangle mapping, 
     *  the index triangles are just the triangles vertex indicies
    */
    std::vector<IndexedTriangleI3> get_index_triangles() {
        std::vector<IndexedTriangleI3> index_vec = {};
        index_vec.reserve(triangles.size());

        for(int i = 0; i < triangles.size(); i++) {
            IndexedTriangleI3 new_triangle = 
                IndexedTriangleI3(i*3+0,i*3+1,i*3+2);
            index_vec.push_back(new_triangle);
        }
        return index_vec;
    }

    static std::vector<VertexBufferElement> get_vertex_buffer_layout() {
        return Triangle::get_vertex_buffer_layout();
    }
};

/**
 * Create 2 triangles to reprent a rectangle of a specified color
*/
static Triangles triangles_from_2d_rect(Rect rect, glm::vec4 color) {
    glm::vec3 p1 = glm::vec3(rect.min_x(), rect.min_y(), 0.0); // (0, 0)
    glm::vec3 p2 = glm::vec3(rect.min_x(), rect.max_y(), 0.0); // (0, 1)
    glm::vec3 p3 = glm::vec3(rect.max_x(), rect.min_y(), 0.0); // (1, 0)
    glm::vec3 p4 = glm::vec3(rect.max_x(), rect.max_y(), 0.0); // (1, 1)

    Triangle triangle1 = Triangle();
    triangle1.set_triangle_color(color);
    triangle1.disable_texture_coord();
    triangle1.set_triangle_position(p1, p2, p4);

    Triangle triangle2 = Triangle();
    triangle2.set_triangle_color(color);
    triangle2.disable_texture_coord();
    triangle2.set_triangle_position(p1, p3, p4);

    Triangles triangls = Triangles();
    triangls.triangles.push_back(triangle1);
    triangls.triangles.push_back(triangle2);
    return triangls;
}

void overwrite_index_vertex_buffer_pair(IndexVertexBufferPair& buffer_pair, Triangles triangles) {
    std::vector<IndexedTriangleI3> index_buffer_triangles = triangles.get_index_triangles();
    buffer_pair.overwrite_data(
    (void*)(&index_buffer_triangles[0]), 
    index_buffer_triangles.size() * sizeof(IndexedTriangleI3), 
    (void*)(&triangles.triangles[0]), 
    triangles.triangles.size() * sizeof(Triangles));
}
IndexVertexBufferPair create_index_vertex_buffer_pair(Triangles triangles) {
    std::vector<IndexedTriangleI3> index_buffer_triangles = triangles.get_index_triangles();
    IndexVertexBufferPair new_buffer_pair = IndexVertexBufferPair::from_raw_data(
        (void*)(&index_buffer_triangles[0]), 
        index_buffer_triangles.size() * sizeof(IndexedTriangleI3), 
        (void*)(&triangles.triangles[0]), 
        triangles.triangles.size() * sizeof(Triangles));

    return new_buffer_pair;
}

class GeneralGPUSetup {
public:
    GPUSetup gpu_setup = GPUSetup();
    void init() {
        // Setup Window
        gpu_setup.window_renderer = WindowRendererPair(1000, 1000);
        create_vertex_array_object();

        // Setup GPU program
        Debug("Add GPU program settings");
        gpu_setup.program.program = GPUProgram::from_strings(vert_shader, frag_shader);
        gpu_setup.program.bind_program();
        //gpu_setup.program.uniform_mat4f_buffer_vars["u_MVP"] = glm::mat4(1.0); // Set view transform to do nothing
        //gpu_setup.program.setup_uniform_buffer_variables();
        
        // Create a basic index vertex buffer pair with a rectangle in the middle of the screen
        gpu_setup.index_vertex_buffer = create_index_vertex_buffer_pair(
            triangles_from_2d_rect(Rect(-0.5, -0.5, 1.0, 1.0), glm::dvec4(1, 0, 0, 1))
        );
        gpu_setup.index_vertex_buffer.bind();
        gpu_setup.vertex_buffer_layout = VertexBufferLayout(Triangles::get_vertex_buffer_layout());
        gpu_setup.vertex_buffer_layout.set_vertex_buffer_layout();

        Debug("gpu_setup.setup()");
        gpu_setup.setup();
    }
    void draw() {
        gpu_setup.draw();
    }
    void show_on_screen() {
        gpu_setup.show_on_screen();
    }
    void set_triangles(Triangles triangles) {
        overwrite_index_vertex_buffer_pair(gpu_setup.index_vertex_buffer, triangles);
    }
    void set_view_transform(CameraViewTransform transform) {
        gpu_setup.program.uniform_mat4f_buffer_vars["u_MVP"] = transform.mat;
        gpu_setup.program.setup_uniform_buffer_variables();
    }
};
}
}
