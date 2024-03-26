#define USE_DEBUG
#define DEBUG_KEYWORDS "." 
#include "../../vicmil_opengl_app.h"

static bool quitting = false;
static vicmil::GPUSetup setup;
vicmil::GPUTexture texture;
float rotation = 0.0;

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

    vicmil::gpu_setup_general::Triangle triangle;
    vicmil::IndexedTriangleI3 traingle_index = vicmil::IndexedTriangleI3(0, 1, 2);

    setup.index_vertex_buffer = vicmil::IndexVertexBufferPair::from_raw_data(
        &traingle_index,
        1 * sizeof(vicmil::IndexedTriangleI3),
        &triangle,
        1 * sizeof(vicmil::gpu_setup_general::Triangle)
    );

    std::vector<vicmil::VertexBufferElement> layout = vicmil::PositionF3TextureCoordF2ColorF3::get_vertex_buffer_layout();
    setup.vertex_buffer_layout = vicmil::VertexBufferLayout(layout);
    setup.vertex_buffer_layout.set_vertex_buffer_layout();

    setup.program.program = vicmil::GPUProgram::from_strings(vicmil::gpu_setup_general::vert_shader, vicmil::gpu_setup_general::frag_shader);
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