#include <SDL2/SDL.h>
#include <stdio.h>
#include "renderer.h"
#include "microui.h"

#include "audio.c"

static float bg[3] = { 90, 95, 100 };


static void main_window(mu_Context *ctx) {
    if (mu_begin_window_ex(ctx, "Main Window", mu_rect(0, 0, 300, 452), MU_OPT_NOCLOSE | MU_OPT_NOTITLE | MU_OPT_NORESIZE | MU_OPT_EXPANDED | MU_OPT_NOSCROLL)) {
        mu_Container *win = mu_get_current_container(ctx);
        win->rect.w = mu_max(win->rect.w, 240);
        win->rect.h = mu_max(win->rect.h, 300);

        int button_width = 143;
        int button_height = 145;

        mu_layout_row(ctx, 2, (int[]) { button_width, button_width }, button_height);
        if (mu_button_ex(ctx, "E", 0, MU_OPT_HOLDFOCUS | MU_OPT_ALIGNCENTER)) {
            printf("Button E pressed\n");
        }
        if (mu_button_ex(ctx, "A", 0, MU_OPT_HOLDFOCUS | MU_OPT_ALIGNCENTER)) { }

        mu_layout_row(ctx, 2, (int[]) { button_width, button_width }, button_height);
        if (mu_button_ex(ctx, "D", 0, MU_OPT_HOLDFOCUS | MU_OPT_ALIGNCENTER)) { }
        if (mu_button_ex(ctx, "G", 0, MU_OPT_HOLDFOCUS | MU_OPT_ALIGNCENTER)) { }

        mu_layout_row(ctx, 2, (int[]) { button_width, button_width }, button_height);
        if (mu_button_ex(ctx, "B", 0, MU_OPT_HOLDFOCUS | MU_OPT_ALIGNCENTER)) { }

        // Since we are naming this button E (same as the low e) we need to set
        // a unique value to attach to the button so it is separate from the
        // low e button
        int high_e = 1;
        mu_push_id(ctx, &high_e, sizeof(high_e));
        if (mu_button_ex(ctx, "E", 0, MU_OPT_HOLDFOCUS | MU_OPT_ALIGNCENTER)) { }
        mu_pop_id(ctx);

        mu_end_window(ctx);
    }
}

static void process_frame(mu_Context *ctx) {
  mu_begin(ctx);
  main_window(ctx);
  mu_end(ctx);
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


static int text_width(mu_Font font, const char *text, int len) {
  if (len == -1) { len = strlen(text); }
  return r_get_text_width(text, len);
}

static int text_height(mu_Font font) {
  return r_get_text_height();
}


int main(int argc, char **argv) {
    /* init SDL and renderer */
    SDL_Init(SDL_INIT_EVERYTHING); // @TODO: Only init what we need
    r_init();

    a_init();

    /* init microui */
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
                    if (b && e.type ==   SDL_MOUSEBUTTONUP) { mu_input_mouseup(ctx, e.button.x, e.button.y, b);   }
                    break;
                }

                case SDL_KEYDOWN:
                case SDL_KEYUP: {
                    int c = key_map[e.key.keysym.sym & 0xff];
                    if (c && e.type == SDL_KEYDOWN) { mu_input_keydown(ctx, c); }
                    if (c && e.type ==   SDL_KEYUP) { mu_input_keyup(ctx, c);   }
                    break;
                }
            }
        }

        /* process frame */
        process_frame(ctx);

        /* render */
        r_clear(mu_color(bg[0], bg[1], bg[2], 255));
        mu_Command *cmd = NULL;
        while (mu_next_command(ctx, &cmd)) {
            switch (cmd->type) {
                case MU_COMMAND_TEXT: r_draw_text(cmd->text.str, cmd->text.pos, cmd->text.color); break;
                case MU_COMMAND_RECT: r_draw_rect(cmd->rect.rect, cmd->rect.color); break;
                case MU_COMMAND_ICON: r_draw_icon(cmd->icon.id, cmd->icon.rect, cmd->icon.color); break;
                case MU_COMMAND_CLIP: r_set_clip_rect(cmd->clip.rect); break;
            }
        }

        r_present();
    }

    //a_cleanup();

    return 0;
}

