#define USE_DEBUG
#define DEBUG_KEYWORDS "." 
#include "../../vicmil_opengl_general.h"

static bool quitting = false;
static vicmil::GPUSetup setup;
vicmil::GPUTexture texture;
float rotation = 0.0;

// Shader sources
const GLchar* vertexSource =
    "#version 130\n"
    "uniform mat4 u_MVP;\n"
    "attribute vec2 position;    \n"
    "attribute vec2 tex_coord;    \n"
    "attribute vec4 color_;    \n"
    "varying vec2 v_tex_coord;\n"   
    "varying vec4 v_Color;\n"
    "void main()                  \n"
    "{                            \n"
    "   v_Color = vec4(color_.x, color_.y, color_.z, 1.0);\n"
    "   v_tex_coord = tex_coord;\n"
    "   gl_Position = u_MVP * vec4(position.xy, 0.0, 1.0);  \n"
    "}                            \n";

const GLchar* fragmentSource =
    "#version 130\n"
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

void render() {
    DisableLogging;
    START_TRACE_FUNCTION();
    setup.window_renderer.set_to_current_window();
    vicmil::clear_screen();

    rotation += 0.001;
    vicmil::Rotation rot = vicmil::Rotation::from_axis_rotation(rotation, glm::dvec3(0, 0, 1));
    vicmil::set_uniform_mat4f(rot.to_matrix4x4(), setup.program.program, "u_MVP");

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
                          0.0f, 0.5f,    0.5, 0.0,  0.0, 0.0, 0.0, 1.0,
                          0.5f, -0.5f,   1.0, 1.0,  1.0, 1.0, 1.0, 1.0,
                          -0.5f, -0.5f,  0.0, 1.0,  1.0, 1.0, 1.0, 1.0,
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
    vicmil::vec_extend(layout, vicmil::ColorF4::get_vertex_buffer_layout());
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