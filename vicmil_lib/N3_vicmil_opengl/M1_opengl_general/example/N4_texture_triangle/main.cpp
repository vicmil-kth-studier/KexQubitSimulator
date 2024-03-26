#define USE_DEBUG
#define DEBUG_KEYWORDS "." 
#include "../../vicmil_opengl_general.h"

static bool quitting = false;
static vicmil::GPUSetup setup;
vicmil::GPUTexture texture;

// Shader sources
const GLchar* vertexSource =
    "#version 130\n"
    "attribute vec2 position;    \n"
    "attribute vec2 tex_coord;    \n"
    "varying vec2 v_tex_coord;\n"   
    "void main()                  \n"
    "{                            \n"
    "   v_tex_coord = tex_coord;\n"
    "   gl_Position = vec4(position.xy, 0.0, 1.0);  \n"
    "}                            \n";

const GLchar* fragmentSource =
    "#version 130\n"
    "precision mediump float;\n"
    "varying vec2 v_tex_coord;\n"
    "uniform sampler2D our_texture;\n"
    "void main()                                  \n"
    "{                                            \n"
    "  gl_FragColor = texture2D(our_texture, v_tex_coord); \n"
    //"gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0); \n"
    "}                                            \n";

void render() {
    DisableLogging;
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
                          0.0f, 0.5f, 0.5, 0.0, // Texture Upper middle
                          0.5f, -0.5f, 1.0, 1.0, // Texture Bottom right
                          -0.5f, -0.5f, 0.0, 1.0, // Texture Bottom left
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
    vicmil::vec_extend(layout, vicmil::TextureCoordF2::get_vertex_buffer_layout());
    setup.vertex_buffer_layout = vicmil::VertexBufferLayout(layout);
    setup.vertex_buffer_layout.set_vertex_buffer_layout();

    setup.program.program = vicmil::GPUProgram::from_strings(vertexSource, fragmentSource);
    setup.program.program.bind_program();

    vicmil::RawImageRGBA image = vicmil::RawImageRGBA();
    image.width = 2;
    image.height = 2;
    image.pixels.resize(2 * 2);
    image.pixels[0] = vicmil::PixelRGBA(0, 0, 255, 255);
    image.pixels[1] = vicmil::PixelRGBA(0, 255, 255, 255);
    image.pixels[2] = vicmil::PixelRGBA(255, 0, 255, 255);
    image.pixels[3] = vicmil::PixelRGBA(255, 255, 255, 255);

    texture = vicmil::GPUTexture::from_raw_image_rgba(image);
    texture.bind();

    // register update as callback
    while(true) {
        update();
    }
}