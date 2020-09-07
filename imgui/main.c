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


// Button colors, make this a struct of colors...

Color background_color = { .r = 30, .g = 30, .b = 30, .a = 255 };
Color black = { .r = 0, .g = 0, .b = 0, .a = 255 };
Color passive = { .r = 50, .g = 50, .b = 50, .a = 255 };
Color hover = { .r = 80, .g = 80, .b = 80, .a = 255 };
Color pressed = { .r = 170, .g = 170, .b = 170, .a = 255 };
Color active = { .r = 150, .g = 150, .b = 150, .a = 255 };

int button_width = 140;
int button_height = 140;
int window_width = 4*3 + 2*140;
int window_height = 4*4 + 3*140;

struct UIState {
    int mouse_x;
    int mouse_y;
    int mouse_down;

    int hot_item;
    int pressed_item; // This is kind of specific to buttons so we might want to handle this on the button itself?
    int active_item;
}
ui_state = {0, 0, 0, 0, 0, 0}; // Global ui state

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
    // Here we clear the active item, we might need to not do that so we can
    // keep buttons active across frames. Either that or we let the caller
    // handle if the button is currently active...
    if (ui_state.mouse_down == 0) {
        ui_state.pressed_item = 0;
    }
}

int region_hit(int x, int y, int w, int h) {
    if (ui_state.mouse_x < x || ui_state.mouse_y < y ||
        ui_state.mouse_x >= x + w || ui_state.mouse_y >= y +h)
        return 0;
    return 1;
}

int button(int id, int x, int y) {
    if (region_hit(x, y, button_width, button_height)) {
        ui_state.hot_item = id;
        if (ui_state.mouse_down) {
            ui_state.pressed_item = id;
        }
    }

    // Render button 
    if (ui_state.hot_item == id) {
        if (ui_state.pressed_item == id) {
            // Button is both hot and pressed
            drawrect(x, y, button_width, button_height, pressed);
        } else {
            // Button is either active or hovered
            if (ui_state.active_item == id) {
                drawrect(x, y, button_width, button_height, active);
            } else {
                drawrect(x, y, button_width, button_height, hover);
            }
        }
    } else {
        // Button is not hovered but could be either active or passive
        if (ui_state.active_item == id) {
            drawrect(x, y, button_width, button_height, active);
        } else {
            drawrect(x, y, button_width, button_height, passive);
        }
    }

    // If button is hot and pressed, but mouse button is not
    // down, the user must have clicked the button.
    if (ui_state.mouse_down == 0 && ui_state.hot_item == id && ui_state.pressed_item == id) {
        // If this button is active clear it, otherwise make it active
        if (ui_state.active_item == id) {
            ui_state.active_item = 0;
        } else {
            ui_state.active_item = id;
        }

        return 1;
    }

    // No click
    return 0;
}

// Rendering function
void render() {   
    // clear screen
    drawrect(0, 0, 640, 480, background_color);

    imgui_prepare();

    if (button(1, 4, 4)) {
        printf("Button1 pressed\n");
    }

    if (button(2, 4*2+button_width, 4)) {
        printf("Button2 pressed\n");
    }

    if (button(3, 4, 4*2+button_height)) {
        printf("Button3 pressed\n");
    }

    if (button(4, 4*2+button_width, 4*2+button_height)) {
        printf("Button4 pressed\n");
    }

    if (button(5, 4, 4*3+2*button_height)) {
        printf("Button3 pressed\n");
    }

    if (button(6, 4*2+button_width, 4*3+2*button_height)) {
        printf("Button4 pressed\n");
    }

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
      window_width, window_height, 0);
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

        // We should probably render after handling all the events...

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
                    if (event.button.button == SDL_BUTTON_LEFT)
                      ui_state.mouse_down = 1;
                    break;
                case SDL_MOUSEBUTTONUP:
                    // update button down state if left-clicking
                    if (event.button.button == SDL_BUTTON_LEFT)
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
                    return 0;
            }
        }
    }

    return 0;
}
