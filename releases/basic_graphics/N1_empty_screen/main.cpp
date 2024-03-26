#define USE_DEBUG
#define DEBUG_KEYWORDS "." 
#include "../../source/quantum_computer_include.h"

using namespace vicmil;

bool init_called = false;


void init() {
    START_TRACE_FUNCTION();

    END_TRACE_FUNCTION();
}

void render() {
    START_TRACE_FUNCTION();
    SDL_GL_MakeCurrent(setup.window, setup.gl_context);

    glClearColor( 1.0f, 0.0f, 1.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    SDL_GL_SwapWindow(setup.window);
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

// Handle emscripten
void native_app() {
    while(true) {
        update();
    }
}
int main(int argc, char *argv[]) {
    Debug("Main!");
    #ifdef __EMSCRIPTEN__
        std::cout << "Emscripten!" << std::endl;
        emscripten_set_main_loop(update, 0, 1);
    #else
        native_app();
    #endif

    return 0;
};

