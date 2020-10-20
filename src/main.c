#include <stdlib.h>
#include <SDL2/SDL.h>

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#include "audio.c"

static SDL_Window *window;
static SDL_Renderer *renderer;

typedef struct {
    int r;
    int g;
    int b;
    int a;
} Color;

typedef struct {
    stbtt_fontinfo* info;
    stbtt_packedchar* chars;
    SDL_Texture* atlas;
    int texture_size;
    float size;
    float scale;
    int ascent;
    int baseline;
} FontData;
FontData* font_data;

void init_font() {
    float size = 30.0;
    FILE *ff = fopen("font/monospace.ttf", "rb");
    if (!ff) { printf("font file not found\n"); exit(1); }

    fseek(ff, 0, SEEK_END);
    int fsize = ftell(ff);
    rewind(ff);
    unsigned char* buffer = malloc(1<<20);
    fread(buffer, 1, fsize, ff);
    fclose(ff);

    font_data = calloc(sizeof(FontData), 1);
    font_data->info = malloc(sizeof(stbtt_fontinfo));
    font_data->chars = malloc(sizeof(stbtt_packedchar) * 96); // ASCII 32..126 is 95 glyphs

    if(stbtt_InitFont(font_data->info, buffer, 0) == 0) {
        free(buffer);
        return;
    }

    // fill bitmap atlas with packed characters
    unsigned char* bitmap = NULL;
    font_data->texture_size = 32;
    while (true) {
        bitmap = malloc(font_data->texture_size * font_data->texture_size);
        stbtt_pack_context pack_context;
        stbtt_PackBegin(&pack_context, bitmap, font_data->texture_size, font_data->texture_size, 0, 1, 0);
        stbtt_PackSetOversampling(&pack_context, 1, 1);
        if(!stbtt_PackFontRange(&pack_context, buffer, 0, size, 32, 95, font_data->chars)) {
            // too small
            free(bitmap);
            stbtt_PackEnd(&pack_context);
            font_data->texture_size *= 2;
        } else {
            stbtt_PackEnd(&pack_context);
            break;
        }
    }

    // convert bitmap to texture
    font_data->atlas = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, font_data->texture_size, font_data->texture_size);
    SDL_SetTextureBlendMode(font_data->atlas, SDL_BLENDMODE_BLEND);

    Uint32* pixels = malloc(font_data->texture_size * font_data->texture_size * sizeof(Uint32));
    static SDL_PixelFormat* format = NULL; // Should this just be made global (at top of file)?
    if (format == NULL) format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
    for (int i = 0; i < font_data->texture_size * font_data->texture_size; i++) {
        pixels[i] = SDL_MapRGBA(format, 0xff, 0xff, 0xff, bitmap[i]);
    }
    SDL_UpdateTexture(font_data->atlas, NULL, pixels, font_data->texture_size * sizeof(Uint32));
    free(pixels);
    free(bitmap);

    // setup additional info
    font_data->scale = stbtt_ScaleForPixelHeight(font_data->info, size);
    stbtt_GetFontVMetrics(font_data->info, &font_data->ascent, 0, 0);
    font_data->baseline = (int) (font_data->ascent * font_data->scale);

    free(buffer);
}

void draw_text(const char *text, int x, int y, Color color) {
    SDL_SetTextureColorMod(font_data->atlas, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(font_data->atlas, color.a);
    for(int i = 0; text[i]; i++) {
        // Check char is within ascii range
        if ((int)text[i] >= 32 && (int)text[i] < 128) {
            stbtt_packedchar* info = &font_data->chars[text[i] - 32];

            SDL_Rect src_rect = {
                info->x0,
                info->y0,
                info->x1 - info->x0,
                info->y1 - info->y0
            };
            SDL_Rect dst_rect = {
                x + info->xoff - (info->x1 - info->x0)/2,
                // @Robustness: Here we center the letter within the button in
                // the y direction. This will not work for multiple characters
                // as each char is a different height, we should update this
                // code to handle centering text properly
                y + info->yoff + (info->y1 - info->y0)/2,
                info->x1 - info->x0,
                info->y1 - info->y0
            };

            SDL_RenderCopy(renderer, font_data->atlas, &src_rect, &dst_rect);
            x += info->xadvance;
        }
    }
}

// Button colors
Color background_color = { .r = 30, .g = 30, .b = 30, .a = 255 };
Color black = { .r = 0, .g = 0, .b = 0, .a = 255 };
Color white = { .r = 255, .g = 255, .b = 255, .a = 255 };
Color red = { .r = 255, .g = 0, .b = 0, .a = 255 };
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
void draw_rect(int x, int y, int w, int h, Color color) {
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

// Prepare ui state
void ui_prepare() {
    ui_state.hot_item = 0;
}

// Finish up after ui processing
void ui_finish() {
    // Here we clear the pressed item if the mouse isn't down
    if (ui_state.mouse_down == 0) {
        ui_state.pressed_item = 0;
    }
}

// Test if given location is within the region
int region_hit(int x, int y, int w, int h) {
    if (ui_state.mouse_x < x || ui_state.mouse_y < y ||
        ui_state.mouse_x >= x + w || ui_state.mouse_y >= y +h)
        return 0;
    return 1;
}

int button(int id, int x, int y, char *text) {
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
            draw_rect(x, y, button_width, button_height, pressed);
            draw_text(text, x+button_height/2, y+button_width/2, white);
        } else {
            // Button is either active or hovered
            if (ui_state.active_item == id) {
                draw_rect(x, y, button_width, button_height, active);
            draw_text(text, x+button_height/2, y+button_width/2, white);
            } else {
                draw_rect(x, y, button_width, button_height, hover);
            draw_text(text, x+button_height/2, y+button_width/2, white);
            }
        }
    } else {
        // Button is not hovered but could be either active or passive
        if (ui_state.active_item == id) {
            draw_rect(x, y, button_width, button_height, active);
            draw_text(text, x+button_height/2, y+button_width/2, white);
        } else {
            draw_rect(x, y, button_width, button_height, passive);
            draw_text(text, x+button_height/2, y+button_width/2, white);
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
    // Clear the screen
    draw_rect(0, 0, window_width, window_height, background_color);

    ui_prepare();

    if (button(1, 4, 4, "E")) {
        a_set_tone(82.41, VOLUME);
    }

    if (button(2, 4*2+button_width, 4, "A")) {
        a_set_tone(110.0, VOLUME);
    }

    if (button(3, 4, 4*2+button_height, "D")) {
        a_set_tone(146.83, VOLUME);
    }

    if (button(4, 4*2+button_width, 4*2+button_height, "G")) {
        a_set_tone(196.0, VOLUME);
    }

    if (button(5, 4, 4*3+2*button_height, "B")) {
        a_set_tone(246.94, VOLUME);
    }

    if (button(6, 4*2+button_width, 4*3+2*button_height, "E")) {
        a_set_tone(329.63, VOLUME);
    }

    if (ui_state.active_item == 0) {
        a_set_tone(0.0, 0.0);
    }

    ui_finish();

    // Update window
    SDL_RenderPresent(renderer);

    // Small delay to reduce unnecessary frame draws
    SDL_Delay(10);
}

int main(int argc, char *argv[]) {
    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO) < 0) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit); // Should we be doing audio cleanup here???

    // Create window
    window = SDL_CreateWindow(
      "Sine Tuner", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      window_width, window_height, 0);
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // If we fail, return error.
    if (window == NULL || renderer == NULL) {
        fprintf(stderr, "Unable to set up window/renderer: %s\n", SDL_GetError());
        exit(1);
    }

    init_font();

    a_init();
    a_start_playing(); // Should this be in the init?

    while (true) {
        // Render stuff
        render();

        // We should probably render after handling all the events...

        // Process user events
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_MOUSEMOTION:
                    // update mouse position
                    ui_state.mouse_x = event.motion.x;
                    ui_state.mouse_y = event.motion.y;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if (event.button.button == SDL_BUTTON_LEFT)
                        ui_state.mouse_down = 1;
                    break;
                case SDL_MOUSEBUTTONUP:
                    if (event.button.button == SDL_BUTTON_LEFT)
                        ui_state.mouse_down = 0;
                    break;
                case SDL_KEYUP:
                    switch (event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            // Quit on escape
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
