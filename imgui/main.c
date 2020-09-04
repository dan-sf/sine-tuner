#include <stdlib.h>
#include <SDL2/SDL.h>

// Screen surface
static SDL_Window *window;
static SDL_Renderer *renderer;

typedef struct {
    int r;
    int g;
    int b;
    int a;
} Color;

struct UIState {
    int mouse_x;
    int mouse_y;
    int mouse_down;

    int hot_item;
    int action_item;
}
ui_state = {0, 0, 0, 0, 0}; // Global ui state

// Simplified interface to SDL's fillrect call
void drawrect(int x, int y, int w, int h, Color color)
{
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;
    SDL_RenderFillRect(renderer, &rect);
}

// Rendering function
void render() {   
    // clear screen
    Color color;
    color.r = 50; color.b = 50; color.g = 50; color.a = 0xff;
    drawrect(0,0,640,480,color);

    // test that the fillrect is working
    color.r = 230; color.b = 230; color.g = 230;
    // drawrect(64,48,64,48,color);

    // testing ui_state
    if (ui_state.mouse_down) { color.b = 0; }
    drawrect(ui_state.mouse_x, ui_state.mouse_y, 64, 48, color);

    // update the screen
    SDL_RenderPresent(renderer);

    // don't take all the cpu time
    SDL_Delay(10); 
}

int region_hit(int x, int y, int w, int h) {
    if (ui_state.mouse_x < x || ui_state.mouse_y < y ||
        ui_state.mouse_x >= x + w || ui_state.mouse_y >= y +h)
        return 0;
    return 1;
}

int button(int id, int x, int y) {
    if (region_hit(x, y, 64, 48)) {
        ui_state.hot_item = id;
        if (ui_state.active_item == 0 && ui_state.mouse_down) {
            ui_state.active_item = id;
        }
    }
}

// Entry point
int main(int argc, char *argv[]) {
    // Initialize SDL's subsystems - in this case, only video.
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }

    // Register SDL_Quit to be called at exit; makes sure things are
    // cleaned up when we quit.
    atexit(SDL_Quit);

    // Attempt to create a 640x480 window with 32bit pixels.

    window = SDL_CreateWindow(
      "ImGui", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      640, 480, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // // If we fail, return error.
    //   if (gScreen == NULL) 
    // {
    //       fprintf(stderr, "Unable to set up video: %s\n", SDL_GetError());
    //       exit(1);
    //   }

    // Main loop: loop forever.
    while (1) {
        // Render stuff
        render();

        // Poll for events, and handle the ones we care about.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_MOUSEMOTION:
                    // update mouse position
                    ui_state.mouse_x = event.motion.x;
                    ui_state.mouse_y = event.motion.y;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    // update button down state if left-clicking
                    if (event.button.button == 1)
                      ui_state.mouse_down = 1;
                    break;
                case SDL_MOUSEBUTTONUP:
                    // update button down state if left-clicking
                    if (event.button.button == 1)
                      ui_state.mouse_down = 0;
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                        // If escape is pressed, return (and thus, quit)
                        return 0;
                    }
                break;
                case SDL_QUIT:
                    return(0);
            }
        }
    }
    return 0;
}
