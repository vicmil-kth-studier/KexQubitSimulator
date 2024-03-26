#define USE_DEBUG
#define DEBUG_KEYWORDS "." 
#include "../../vicmil_opengl_general.h"

using namespace vicmil;

bool init_called = false;
static WindowRendererPair window_and_renderer;

void render() {
    START_TRACE_FUNCTION();

    assert(GLClearErrors() != -1);
    glClearColor( 1.0f, 0.0f, 1.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    SDL_GL_MakeCurrent(window_and_renderer.window, window_and_renderer.gl_context);
    SDL_GL_SwapWindow(window_and_renderer.window);
    END_TRACE_FUNCTION();
}

void update(){
    SDL_Event event;
    while( SDL_PollEvent(&event) ) {
        if(event.type == SDL_QUIT) {
            throw;
        }
    }

    render();
};

int main(int argc, char *argv[]) {
    Debug("Main!");
    window_and_renderer = WindowRendererPair(500, 500);
    create_vertex_array_object();
    while(true) {
        update();
    }

    return 0;
};

