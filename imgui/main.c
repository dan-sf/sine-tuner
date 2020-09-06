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
Color gray1 = { .r = 80, .g = 80, .b = 80, .a = 80 };
Color gray2 = { .r = 170, .g = 170, .b = 170, .a = 170 };
Color black = { .r = 0, .g = 0, .b = 0, .a = 0 };
Color background_color = { .r = 150, .g = 150, .b = 240, .a = 0 };

// Button colors, make this a struct of colors...

Color passive = { .r = 50, .g = 50, .b = 50, .a = 50 };
Color hover = { .r = 80, .g = 80, .b = 80, .a = 80 };
Color pressed = { .r = 170, .g = 170, .b = 170, .a = 170 };
Color active = { .r = 225, .g = 225, .b = 225, .a = 225 };

// off/passive
// hot/hovered
// pressed (mouse down on the button)
// active/on (button is in the on state)



struct UIState {
    int mouse_x;
    int mouse_y;
    int mouse_down;

    int hot_item;
    int pressed_item; // This is kind of specific to buttons so we might want to handle this on the button itself?
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

int button(int id, int x, int y, int is_active) {
    if (region_hit(x, y, 64, 48)) {
        ui_state.hot_item = id;
        if (ui_state.mouse_down) {
            ui_state.pressed_item = id;
        }
    }

    // Render button 
    // drawrect(x+8, y+8, 64, 48, black);
    if (ui_state.hot_item == id) {
        if (ui_state.pressed_item == id) {
            // Button is both 'hot' and 'active'
            //if (is_active) drawrect(x, y, 64, 48, hover);
            //else drawrect(x, y, 64, 48, active);
            drawrect(x, y, 64, 48, pressed);
        } else {
            // Button is merely 'hot'
            if (is_active)
                drawrect(x, y, 64, 48, active);
            else
                drawrect(x, y, 64, 48, hover);
        }
    } else {
        // button is not hot, but it may be active    
        if (is_active) drawrect(x, y, 64, 48, active);
        else drawrect(x, y, 64, 48, passive);
    }

  // If button is hot and active, but mouse button is not
  // down, the user must have clicked the button.
    if (ui_state.mouse_down == 0 && ui_state.hot_item == id && ui_state.pressed_item == id)
        return 1;

    // Otherwise, no clicky.
    return 0;
}

// Store global active state for the buttons
static int button1_active = 0;
static int button2_active = 0;
static int button3_active = 0;
static int button4_active = 0;

// Rendering function
void render() {   
    // clear screen
    drawrect(0, 0, 640, 480, background_color);

    imgui_prepare();

    // There is probably a better way to handle the button active state, we
    // might want to put that into an array and have the indices map to button
    // ids... We could then have a toggle_active function that only needs the
    // id of the button

    if (button(1, 50, 50, button1_active)) {
        printf("Button1 pressed\n");
        button1_active ^= 1;
        button2_active = 0;
        button3_active = 0;
        button4_active = 0;
    }

    if (button(2, 150, 50, button2_active)) {
        printf("Button2 pressed\n");
        button1_active = 0;
        button2_active ^= 1;
        button3_active = 0;
        button4_active = 0;
    }

    if (button(3, 50, 150, button3_active)) {
        printf("Button3 pressed\n");
        button1_active = 0;
        button2_active = 0;
        button3_active ^= 1;
        button4_active = 0;
    }

    if (button(4, 150, 150, button4_active)) {
        printf("Button4 pressed\n");
        button1_active = 0;
        button2_active = 0;
        button3_active = 0;
        button4_active ^= 1;
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
