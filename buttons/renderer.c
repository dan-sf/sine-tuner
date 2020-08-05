#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>
#include <assert.h>
#include "renderer.h"
#include "atlas.inl"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define BUFFER_SIZE 16384

static float   tex_buf[BUFFER_SIZE *  8];
static float  vert_buf[BUFFER_SIZE *  8];
static uint8_t color_buf[BUFFER_SIZE * 16];
static uint32_t  index_buf[BUFFER_SIZE *  6];

static char ttf_buffer[BUFFER_SIZE * 32];
static stbtt_fontinfo font;

static int width  = 300;
static int height = 452;
static int buf_idx;

static SDL_Window *window;
static SDL_Renderer *renderer;


void r_init(void) {

   // FILE *ff = fopen("font.ttf", "rb");
   // if (!ff) { printf("font file not found\n"); exit(1); }
   // fseek(ff, 0, SEEK_END);
   // int fsize = ftell(ff);
   // rewind(ff);
   // fread(ttf_buffer, 1, fsize, ff);

   // stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));





  /* init SDL window */
  window = SDL_CreateWindow(
    "Sine Tuner", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
    width, height, 0);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  r_init_font();
}


void r_init_font(void) {
   FILE *ff = fopen("font.ttf", "rb");
   if (!ff) { printf("font file not found\n"); exit(1); }
   fseek(ff, 0, SEEK_END);
   int fsize = ftell(ff);
   rewind(ff);
   fread(ttf_buffer, 1, fsize, ff);

   stbtt_InitFont(&font, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer,0));
}


static void flush(void) { // I'm not sure if this flush is needed?
  if (buf_idx == 0) { return; }
  buf_idx = 0;
}

static void push_quad(mu_Rect dst, mu_Rect src, mu_Color color) { // I'm not sure if push quad is needed anymore
  if (buf_idx == BUFFER_SIZE) { flush(); }

  int texvert_idx = buf_idx *  8;
  int   color_idx = buf_idx * 16;
  int element_idx = buf_idx *  4;
  int   index_idx = buf_idx *  6;
  buf_idx++;

  /* update texture buffer */
  float x = src.x / (float) ATLAS_WIDTH;
  float y = src.y / (float) ATLAS_HEIGHT;
  float w = src.w / (float) ATLAS_WIDTH;
  float h = src.h / (float) ATLAS_HEIGHT;
  tex_buf[texvert_idx + 0] = x;
  tex_buf[texvert_idx + 1] = y;
  tex_buf[texvert_idx + 2] = x + w;
  tex_buf[texvert_idx + 3] = y;
  tex_buf[texvert_idx + 4] = x;
  tex_buf[texvert_idx + 5] = y + h;
  tex_buf[texvert_idx + 6] = x + w;
  tex_buf[texvert_idx + 7] = y + h;

  /* update vertex buffer */
  vert_buf[texvert_idx + 0] = dst.x;
  vert_buf[texvert_idx + 1] = dst.y;
  vert_buf[texvert_idx + 2] = dst.x + dst.w;
  vert_buf[texvert_idx + 3] = dst.y;
  vert_buf[texvert_idx + 4] = dst.x;
  vert_buf[texvert_idx + 5] = dst.y + dst.h;
  vert_buf[texvert_idx + 6] = dst.x + dst.w;
  vert_buf[texvert_idx + 7] = dst.y + dst.h;

  /* update color buffer */
  memcpy(color_buf + color_idx +  0, &color, 4);
  memcpy(color_buf + color_idx +  4, &color, 4);
  memcpy(color_buf + color_idx +  8, &color, 4);
  memcpy(color_buf + color_idx + 12, &color, 4);

  /* update index buffer */
  index_buf[index_idx + 0] = element_idx + 0;
  index_buf[index_idx + 1] = element_idx + 1;
  index_buf[index_idx + 2] = element_idx + 2;
  index_buf[index_idx + 3] = element_idx + 2;
  index_buf[index_idx + 4] = element_idx + 3;
  index_buf[index_idx + 5] = element_idx + 1;
}

void r_draw_rect(mu_Rect rect, mu_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    SDL_Rect sdl_rect;
    sdl_rect.x = rect.x;
    sdl_rect.y = rect.y;
    sdl_rect.w = rect.w;
    sdl_rect.h = rect.h;

    SDL_RenderFillRect(renderer, &sdl_rect);
}


void r_draw_text(const char *text, mu_Vec2 pos, mu_Color color) {

   unsigned char *bitmap = 0;

   int font_size = 20;
   float scale = stbtt_ScaleForPixelHeight(&font, font_size);
   int ascent, descent;
   stbtt_GetFontVMetrics(&font, &ascent, &descent, 0);

   int ind,
       advance,
       h,w,
       xpos=pos.x,
       ypos=pos.y+scale*ascent;
   // printf("ascent=%d descent=%d linedist=%d\n",(int)(scale*ascent),(int)(scale*descent),(int)(scale * (ascent-descent)));
   int ix0, iy0, ix1, iy1;
   for (ind=0; text[ind]; ++ind) {
      bitmap = stbtt_GetCodepointBitmap(&font, scale, scale, text[ind], &w, &h, 0, 0);
      stbtt_GetCodepointHMetrics(&font, text[ind], &advance, 0);
      stbtt_GetCodepointBitmapBox(&font, text[ind], scale, scale, &ix0, &iy0, &ix1, &iy1);

      int i,j,
          x=xpos,
          y=ypos;
      for (j=0; j < h; ++j) {
         for (i=0; i < w; ++i) {
            if (bitmap[j*w+i]) {
              SDL_SetRenderDrawColor(renderer, 0, 0, 0, bitmap[j*w+i]);
              SDL_RenderDrawPoint(renderer, x+ix0, y+iy0);
            }
            ++x;
         }
         x=xpos;
         ++y;
      }
      stbtt_FreeBitmap(bitmap, 0);
      if (!text[ind+1]) break;
      xpos += scale*advance;
   }

   // SDL_RenderPresent(renderer);
   // SDL_Event event;
   // while (1)
   //   while(SDL_PollEvent(&event))
   //     if (event.type==SDL_QUIT) { SDL_Quit(); exit(0); }
   // return 0;



  // mu_Rect dst = { pos.x, pos.y, 0, 0 };
  // for (const char *p = text; *p; p++) {
  //   if ((*p & 0xc0) == 0x80) { continue; }
  //   int chr = mu_min((unsigned char) *p, 127);
  //   mu_Rect src = atlas[ATLAS_FONT + chr];
  //   dst.w = src.w;
  //   dst.h = src.h;
  //   push_quad(dst, src, color);
  //   dst.x += dst.w;
  // }
}

void r_draw_icon(int id, mu_Rect rect, mu_Color color) {
  mu_Rect src = atlas[id];
  int x = rect.x + (rect.w - src.w) / 2;
  int y = rect.y + (rect.h - src.h) / 2;
  push_quad(mu_rect(x, y, src.w, src.h), src, color);
}

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

void r_set_clip_rect(mu_Rect rect) {
  flush();
    SDL_Rect sdl_rect;
    sdl_rect.x = rect.x;
    sdl_rect.y = rect.y;
    sdl_rect.w = rect.w;
    sdl_rect.h = rect.h;
  SDL_RenderSetClipRect(renderer, &sdl_rect);
}

void r_clear(mu_Color clr) {
    flush();
    SDL_SetRenderDrawColor(renderer, clr.r, clr.g, clr.b, clr.a);
    SDL_RenderClear(renderer);
}


void r_present(void) {
    flush();
    SDL_RenderPresent(renderer);
}
