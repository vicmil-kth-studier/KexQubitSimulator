#include "L4_gpu_texture.h"

namespace vicmil {
void init_SDL() {
    if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)) { 
        ThrowError("Could not initialize SDL: %s.\n" << SDL_GetError());
    }
}

void quit_SDL() {
    SDL_Quit();
}

/**
 * Quit if requested by user
 * Update window to keep it alive
*/ 
std::vector<SDL_Event> update_SDL() {
    std::vector<SDL_Event> events;
    SDL_Event event;
    while( SDL_PollEvent( &event ) ) {
        if( event.type == SDL_QUIT )
        {
            quit_SDL();
            throw;
        }
        events.push_back(event);
    }
    return events;
}

/**
 * Using blening in the frag shader makes the graphics look better by being less blocky
*/
void setup_frag_shader_blending() {
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
}

void create_window_and_renderer(int width, int height, SDL_Renderer** renderer, SDL_Window** window) {
    /* Create a windowed mode window and its OpenGL context */
    #ifdef __EMSCRIPTEN__
    // Resizing in emscripten makes the window flicker :(
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_OPENGL, window, renderer);
    #else
    SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE, window, renderer);
    #endif

    Debug("OpenGL version " << glGetString(GL_VERSION));
}

class WindowRendererPair {
public:
    SDL_Renderer *renderer = NULL;
    SDL_Window *window = NULL;
    SDL_GLContext* gl_context = NULL;

    WindowRendererPair() {}
    WindowRendererPair(int width, int height) {
        gl_context = new SDL_GLContext();
        vicmil::create_window_and_renderer(width, height, &renderer, &window);
    }
    void set_to_current_window() {
        SDL_GL_MakeCurrent(window, gl_context);
    }
};

void get_window_size(SDL_Window *window, int* w, int* h) {
    SDL_GL_GetDrawableSize(window, w, h);
}

void clear_screen(glm::vec4 clear_color = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)) {
    glClearColor(clear_color[0], clear_color[1], clear_color[2], clear_color[3]);
    GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

void set_depth_testing_enabled(bool is_enabled) {
    // This makes sure that things closer block things further away
    if(is_enabled) {
        // Enable depth buffer
        glEnable(GL_DEPTH_TEST);
    }
    else {
        glDisable(GL_DEPTH_TEST);
    }
}

/**
 * Create a vertex array object, there usually have to exist one of those for the program to work!
*/
GLuint create_vertex_array_object() {
    GLuint vao;
    glGenVertexArraysOES(1, &vao);
    glBindVertexArrayOES(vao);
    return vao;
}

/**
 * Setup all necessary parameters to draw on the GPU, then draw
*/
class GPUSetup {
public:
    WindowRendererPair window_renderer; // The window and renderer that will be used for the rendering
    GPUProgramWithUniformBuffer program; // The GPU program and any associated uniform buffer variables
    GPUTexture texture; // [OPTIONAL] The texture to use when drawing from

    // Things regarding the buffer used on the GPU(eg what triangles to use and what data each triangles contains)
    IndexVertexBufferPair index_vertex_buffer; // The buffer that contains the data that will be drawn, for example triangle positions and colors
    VertexBufferLayout vertex_buffer_layout; // The layout of the index_vertex_buffer

    // Allow the user to only draw part of the GPU buffer
    int buffer_offset_in_bytes = 0; // [OPTIONAL]
    int buffer_use_triangle_count = -1; // [OPTIONAL] The number of triangles to use after the offset

    // Other settings...
    bool depth_testing = false; // [OPTIONAL] If closer things should block things further back, eg. 3D graphics
    
    void setup() {
        START_TRACE_FUNCTION();
        Debug("window_renderer.set_to_current_window()");
        window_renderer.set_to_current_window();
        Debug("program.bind_and_setup_shader_variables()");
        program.bind_and_setup_shader_variables();
        Debug("texture.bind()");
        texture.bind();
        Debug("index_vertex_buffer.bind()");
        index_vertex_buffer.bind();
        Debug("vertex_buffer_layout.set_vertex_buffer_layout()");
        vertex_buffer_layout.set_vertex_buffer_layout();
        Debug("set_depth_testing_enabled(depth_testing)");
        set_depth_testing_enabled(depth_testing);
        END_TRACE_FUNCTION();
    }
    void set_window_renderer(WindowRendererPair window_renderer_) {
        window_renderer = window_renderer_;
        window_renderer.set_to_current_window();
    }
    void set_gpu_program(GPUProgramWithUniformBuffer program_) {
        program = program_;
        program.bind_and_setup_shader_variables();
    }
    void set_gpu_texture(GPUTexture texture_) {
        texture = texture_;
        texture.bind();
    }
    void set_index_vertex_buffer(IndexVertexBufferPair index_vertex_buffer_) {
        index_vertex_buffer = index_vertex_buffer_;
        index_vertex_buffer.bind();
    }
    void set_vertex_buffer_layout(VertexBufferLayout vertex_buffer_layout_) {
        vertex_buffer_layout = vertex_buffer_layout_;
        vertex_buffer_layout.set_vertex_buffer_layout();
    }
    void set_depth_testing_enabled(bool depth_testing_) {
        depth_testing = depth_testing_;
        vicmil::set_depth_testing_enabled(depth_testing);
    }
    /** Perform the drawing
     * NOTE! need to call show_on_screen() before anything shows up on screen
    */
    void draw() {
        // 
        index_vertex_buffer.draw(buffer_use_triangle_count, buffer_offset_in_bytes);
    }
    void show_on_screen() {
        SDL_GL_SwapWindow(window_renderer.window);
    }
};
}