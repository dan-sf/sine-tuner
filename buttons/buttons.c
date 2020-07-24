#include <SDL2/SDL.h>
#include <stdio.h>
#include "microui.h"
#include "atlas.inl"


static void test_window(mu_Context *ctx) {
  /* do window */
  if (mu_begin_window_ex(ctx, "Demo Window", mu_rect(0, 0, 300, 450), MU_OPT_NOCLOSE | MU_OPT_NOTITLE)) {
    mu_Container *win = mu_get_current_container(ctx);
    win->rect.w = mu_max(win->rect.w, 240);
    win->rect.h = mu_max(win->rect.h, 300);

      mu_layout_row(ctx, 2, (int[]) { 144, 144 }, 100);
      if (mu_button(ctx, "Button 1")) { /* write_log("Pressed button 1"); */ }
      if (mu_button(ctx, "Button 2")) { /* write_log("Pressed button 2"); */ }
      mu_layout_row(ctx, 2, (int[]) { 144, 144 }, 100);
      if (mu_button(ctx, "Button 3")) { /* write_log("Pressed button 3"); */ }
      if (mu_button(ctx, "Button 4")) { /* write_log("Pressed button 4"); */ }
      mu_layout_row(ctx, 2, (int[]) { 144, 144 }, 100);
      if (mu_button(ctx, "Button 5")) { /* write_log("Pressed button 5"); */ }
      if (mu_button(ctx, "Button 6")) { /* printf("Pressed button 6"); */ }

    mu_end_window(ctx);
  }
}

static const char button_map[256] = {
  [ SDL_BUTTON_LEFT   & 0xff ] =  MU_MOUSE_LEFT,
  [ SDL_BUTTON_RIGHT  & 0xff ] =  MU_MOUSE_RIGHT,
  [ SDL_BUTTON_MIDDLE & 0xff ] =  MU_MOUSE_MIDDLE,
};

static const char key_map[256] = {
  [ SDLK_LSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_RSHIFT       & 0xff ] = MU_KEY_SHIFT,
  [ SDLK_LCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_RCTRL        & 0xff ] = MU_KEY_CTRL,
  [ SDLK_LALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RALT         & 0xff ] = MU_KEY_ALT,
  [ SDLK_RETURN       & 0xff ] = MU_KEY_RETURN,
  [ SDLK_BACKSPACE    & 0xff ] = MU_KEY_BACKSPACE,
};

int r_get_text_width(const char *text, int len) {
  int res = 0;
  for (const char *p = text; *p && len--; p++) {
    if ((*p & 0xc0) == 0x80) { continue; }
    int chr = mu_min((unsigned char) *p, 127);
    res += atlas[ATLAS_FONT + chr].w;
  }
  return res;
}

int r_get_text_height(void) {
  return 18;
}

static int text_width(mu_Font font, const char *text, int len) {
    if (len == -1) { len = strlen(text); }
    return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
    return r_get_text_height();
}


int main() {
    // SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);
    SDL_EnableScreenSaver();
    atexit(SDL_Quit);

    SDL_DisplayMode dm;
    SDL_GetCurrentDisplayMode(0, &dm);

    int width  = 300;
    int height = 450;

    SDL_Window *window;
    window = SDL_CreateWindow(
        "Test microui", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI
    );

    if (window == NULL) {
        // In the case that the window could not be made...
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    mu_Context *ctx = malloc(sizeof(mu_Context));
    mu_init(ctx);
    ctx->text_width = text_width;
    ctx->text_height = text_height;

    /* main loop */
    for (;;) {
        /* handle SDL events */
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_QUIT: exit(EXIT_SUCCESS); break;
                case SDL_MOUSEMOTION: mu_input_mousemove(ctx, e.motion.x, e.motion.y); break;
                case SDL_MOUSEWHEEL: mu_input_scroll(ctx, 0, e.wheel.y * -30); break;
                case SDL_TEXTINPUT: mu_input_text(ctx, e.text.text); break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP: {
                    int b = button_map[e.button.button & 0xff];
                    if (b && e.type == SDL_MOUSEBUTTONDOWN) { mu_input_mousedown(ctx, e.button.x, e.button.y, b); }
                    if (b && e.type == SDL_MOUSEBUTTONUP) { mu_input_mouseup(ctx, e.button.x, e.button.y, b); }
                    break;
                }

                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                    int c = key_map[e.key.keysym.sym & 0xff];
                    if (c && e.type == SDL_KEYDOWN) { mu_input_keydown(ctx, c); }
                    if (c && e.type == SDL_KEYUP) { mu_input_keyup(ctx, c); }
                    break;
                }
            }
        }

        //process_frame(ctx);
        mu_begin(ctx);
        test_window(ctx);
        SDL_RenderClear(renderer);
        mu_Command *cmd = NULL;
        while (mu_next_command(ctx, &cmd)) {
            switch (cmd->type) {
                case MU_COMMAND_TEXT:
                    printf("MU_COMMAND_TEXT\n"); break;
                    //r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
                case MU_COMMAND_RECT:
                    printf("MU_COMMAND_RECT\n"); break;
                    //r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
                case MU_COMMAND_ICON:
                    printf("MU_COMMAND_ICON\n"); break;
                    //r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
                case MU_COMMAND_CLIP:
                    printf("MU_COMMAND_CLIP\n"); break;
                    //r_set_clip_rect(cmd->clip.rect); break;
            }
        }

        //r_clear(mu_color(bg[0], bg[1], bg[2], 255));
        //while (mu_next_command(ctx, &cmd)) {
        //    switch (cmd->type) {
        //    case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
        //    case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
        //    case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
        //    case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
        //    }
        //}
        //r_present();
    }

    printf("testing\n");
}
