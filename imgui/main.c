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

Color white = { .r = 255, .g = 255, .b = 255, .a = 255 };
Color gray = { .r = 170, .g = 170, .b = 170, .a = 170 };
Color black = { .r = 0, .g = 0, .b = 0, .a = 0 };
Color background_color = { .r = 150, .g = 150, .b = 240, .a = 0 };

struct UIState {
    int mouse_x;
    int mouse_y;
    int mouse_down;

    int hot_item;
    int active_item;
}
ui_state = {0, 0, 0, 0, 0}; // Global ui state

// Simplified interface to SDL's fillrect call
void drawrect(int x, int y, int w, int h, Color color)
{
    SDL_Rect rect;
    rect.x = x;
    rect.y = y;
    rect.w = w;
    rect.h = h;

    SDL_Rect brect;
    brect.x = x;
    brect.y = y;
    brect.w = w+4;
    brect.h = h+4;

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(renderer, &rect);
    SDL_SetRenderDrawColor(renderer, black.r, black.g, black.b, black.a);
    SDL_RenderDrawRect(renderer, &rect);
}

// Prepare for IMGUI code
void imgui_prepare() {
    ui_state.hot_item = 0;
}

// Finish up after IMGUI code
void imgui_finish() {
    if (ui_state.mouse_down == 0) {
        ui_state.active_item = 0;
    } else {
        if (ui_state.active_item == 0) ui_state.active_item = -1;
    }
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

    // Render button 
    drawrect(x+8, y+8, 64, 48, black);
    if (ui_state.hot_item == id) {
        if (ui_state.active_item == id) {
            // Button is both 'hot' and 'active'
            drawrect(x+2, y+2, 64, 48, white);
        } else {
            // Button is merely 'hot'
            drawrect(x, y, 64, 48, white);
        }
    } else {
        // button is not hot, but it may be active    
        drawrect(x, y, 64, 48, gray);
    }

  // If button is hot and active, but mouse button is not
  // down, the user must have clicked the button.
    if (ui_state.mouse_down == 0 && ui_state.hot_item == id && ui_state.active_item == id)
        return 1;

    // Otherwise, no clicky.
    return 0;
}

// Rendering function
void render() {   
    // clear screen
    drawrect(0,0,640,480,background_color);

    imgui_prepare();

    button(1, 50, 50);
    button(2, 150, 50);

    if (button(3, 50, 150)) {
        background_color.r = 500;
    }

    if (button(4, 150, 150))
        exit(0);

    imgui_finish();

    // update the screen
    SDL_RenderPresent(renderer);

    // don't take all the cpu time
    SDL_Delay(10); 
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

    // If we fail, return error.
    if (window == NULL || renderer == NULL) {
        fprintf(stderr, "Unable to set up window/renderer: %s\n", SDL_GetError());
        exit(1);
    }

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
