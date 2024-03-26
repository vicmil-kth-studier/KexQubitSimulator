#define USE_DEBUG
#define DEBUG_KEYWORDS "." 
#include "../../vicmil_opengl_general.h"

static bool quitting = false;
static vicmil::GPUSetup setup;

// Shader sources
const GLchar* vertexSource =
    "#version 130\n"
    "attribute vec2 position;    \n"
    "attribute vec3 color_;    \n"
    "varying vec4 v_Color;\n"   
    "void main()                  \n"
    "{                            \n"
    "   v_Color = vec4(color_.x, color_.y, color_.z, 1.0);\n"
    "   gl_Position = vec4(position.xy, 0.0, 1.0);  \n"
    "}                            \n";

const GLchar* fragmentSource =
    "#version 130\n"
    "precision mediump float;\n"
    "varying vec4 v_Color;\n"
    "void main()                                  \n"
    "{                                            \n"
    "  gl_FragColor = v_Color; \n"
    "}                                            \n";

void render() {
    START_TRACE_FUNCTION();
    setup.window_renderer.set_to_current_window();
    vicmil::clear_screen();

    // Draw a triangle from the 3 vertices
    setup.draw();

    setup.show_on_screen();
    END_TRACE_FUNCTION();
}

void update(){
    vicmil::update_SDL();
    render();
};

int main(int argc, char *argv[]) {
    Debug("Start!");
    setup.window_renderer = vicmil::WindowRendererPair(1024, 1024);
    vicmil::create_vertex_array_object();

    std::vector<GLfloat> vertices = std::vector<GLfloat>({
                          0.0f, 0.5f, 1.0, 0.0, 0.0,
                          0.5f, -0.5f, 0.0, 1.0, 0.0,
                          -0.5f, -0.5f, 0.0, 0.0, 1.0,
    });
    int indices[] = {0, 1, 2};

    setup.index_vertex_buffer = vicmil::IndexVertexBufferPair::from_raw_data(
        &indices[0],
        3 * sizeof(int),
        &vertices[0],
        vertices.size() * sizeof(GLfloat)
    );

    std::vector<vicmil::VertexBufferElement> layout = {};
    vicmil::vec_extend(layout, vicmil::PositionF2::get_vertex_buffer_layout());
    vicmil::vec_extend(layout, vicmil::ColorF3::get_vertex_buffer_layout());
    setup.vertex_buffer_layout = vicmil::VertexBufferLayout(layout);
    setup.vertex_buffer_layout.set_vertex_buffer_layout();

    setup.program.program = vicmil::GPUProgram::from_strings(vertexSource, fragmentSource);
    setup.program.program.bind_program();

    // register update as callback
    while(true) {
        update();
    }
}