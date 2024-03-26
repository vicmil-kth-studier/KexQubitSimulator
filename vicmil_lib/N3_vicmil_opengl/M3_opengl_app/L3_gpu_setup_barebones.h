#include "L2_view_transform.h"

namespace vicmil {
namespace gpu_setup_barebones {
/**
 * This is an example of a vertex-fragment shader pair that are the most basic we can get away with
 * (Just draw trinagles of single color)
 * 
 * Can be used for debugging or like as an example
*/
const std::string vert_shader =
vicmil::SHADER_VERSON_OPENGL_ES +
"attribute vec2 position;\n"  // The position of the vertex(triangle corner)
"\n"
"void main() {\n"            
"    gl_Position = vec4(position.x, position.y, 0.0, 1.0);\n"
"}\n";

const std::string frag_shader =
vicmil::SHADER_VERSON_OPENGL_ES + 
"precision mediump float;\n"
"\n"
"void main() {\n"
"    gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
"}\n";

IndexVertexBufferPair create_index_vertex_buffer_pair() {
    std::vector<float> vertices = {0.0f, 0.5f, 3.0f,
                0.5f, -0.5f, 3.0f,
                -0.5f, -0.5f, 3.0f};
    std::vector<unsigned int> faces = {1, 2, 3};

    vicmil::IndexVertexBufferPair new_buffer_pair = vicmil::IndexVertexBufferPair::from_raw_data(
        &faces[0], 
        3*sizeof(unsigned int), 
        &vertices[0],
        9 * sizeof(float)
    );

    return new_buffer_pair;
}

class BareBonesGPUSetup {
public:
    GPUSetup gpu_setup = GPUSetup();
    void init() {
        // Setup Window
        gpu_setup.window_renderer = WindowRendererPair(800, 600);

        // Setup GPU program
        Debug("Add GPU program settings");
        gpu_setup.program.program = GPUProgram::from_strings(vert_shader, frag_shader);
        
        // Create a basic index vertex buffer pair with a rectangle in the middle of the screen
        gpu_setup.index_vertex_buffer = create_index_vertex_buffer_pair();
        gpu_setup.vertex_buffer_layout = VertexBufferLayout(PositionF2::get_vertex_buffer_layout());

        Debug("gpu_setup.setup()");
        gpu_setup.setup();
    }
    void draw() {
        gpu_setup.draw();
    }
    void show_on_screen() {
        gpu_setup.show_on_screen();
    }
};
}
}
