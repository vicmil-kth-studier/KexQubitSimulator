#include "L3_buffer.h"

namespace vicmil {
void init_SDL() {
    if((SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)==-1)) { 
        ThrowError("Could not initialize SDL: %s.\n" << SDL_GetError());
    }
}
void quit_SDL() {
    SDL_Quit();
}
void update_SDL() {
    // Quit if requested by user
    SDL_Event event;
    while( SDL_PollEvent( &event ) ) {
        if( event.type == SDL_QUIT )
        {
            quit_SDL();
            throw;
        }
    }
}
    
class GraphicsSetup {
public:
    SDL_Renderer *renderer = NULL;
    SDL_Window *window = NULL;
    SDL_GLContext gl_context;
    int width; // Width in the number of pixels
    int height; // Height in the number of pixels
    double custom_screen_aspect_ratio = -1; // -1 means just use the width and height in pixels as a guide

    GraphicsSetup() {}
    static GraphicsSetup create_setup(int width_ = 1024, int height_ = 1024) {
        GraphicsSetup setup = GraphicsSetup();
        setup.width = width_;
        setup.height = height_;
        setup.create_window_and_renderer();
        //setup.setup_frag_shader_blending();
        return setup;
    }
    static void setup_frag_shader_blending() {
        // Enable blending, which makes textures look less blocky
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
    }
    void create_window_and_renderer() {
        /* Create a windowed mode window and its OpenGL context */
        SDL_CreateWindowAndRenderer(width, height, SDL_WINDOW_OPENGL, &window, &renderer);

        Debug("OpenGL version " << glGetString(GL_VERSION));
    }
    void get_window_size(int* w, int* h) {
        SDL_GL_GetDrawableSize(window, w, h);
        Assert(*w == width);
        Assert(*h == height);
    }
    double get_window_aspect_ratio() {
        if(custom_screen_aspect_ratio > 0) {
            return custom_screen_aspect_ratio;
        }
        return ((double)width) / height;
    }
    ~GraphicsSetup() {
    }
};
}