#include <SDL2/SDL.h>
//#include <SDL2/SDL_opengl.h>
#include <assert.h>
#include "renderer.h"
#include "atlas.inl"

#define STB_TRUETYPE_IMPLEMENTATION
#include "stb_truetype.h"

#define BUFFER_SIZE 16384




unsigned char ttf_buffer[1<<20];
unsigned char temp_bitmap[512*512];

stbtt_bakedchar cdata[96]; // ASCII 32..126 is 95 glyphs




static float   tex_buf[BUFFER_SIZE *  8];
static float  vert_buf[BUFFER_SIZE *  8];
static uint8_t color_buf[BUFFER_SIZE * 16];
static uint32_t  index_buf[BUFFER_SIZE *  6];

//static char ttf_buffer[BUFFER_SIZE * 32];
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
  //r_init_font();
  r_init_font2();
}


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


void r_init_font2() {
    float size = 30.0;
    FILE *ff = fopen("monospace.ttf", "rb");
    if (!ff) { printf("font file not found\n"); exit(1); }
    fseek(ff, 0, SEEK_END);
    int fsize = ftell(ff);
    rewind(ff);
    unsigned char* buffer = malloc(1<<20);
    fread(buffer, 1, fsize, ff);

    // TODO close file handle

    font_data = calloc(sizeof(FontData), 1);
    font_data->info = malloc(sizeof(stbtt_fontinfo)); // use calloc here?
    font_data->chars = malloc(sizeof(stbtt_packedchar) * 96);

    if(stbtt_InitFont(font_data->info, buffer, 0) == 0) {
        free(buffer);
        return;
    }

    // fill bitmap atlas with packed characters
    unsigned char* bitmap = NULL;
    font_data->texture_size = 32;
    while(1) {
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
    static SDL_PixelFormat* format = NULL;
    if(format == NULL) format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);
    for(int i = 0; i < font_data->texture_size * font_data->texture_size; i++) {
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




void r_init_font(void) {
    FILE *ff = fopen("font.ttf", "rb");
    if (!ff) { printf("font file not found\n"); exit(1); }
    fseek(ff, 0, SEEK_END);
    int fsize = ftell(ff);
    rewind(ff);
    fread(ttf_buffer, 1, fsize, ff);

    // FontInfo* font_info = calloc(sizeof(FontInfo), 1);
    // font_info->info = malloc(sizeof(stbtt_fontinfo));
    // font_info->chars = malloc(sizeof(stbtt_packedchar) * 96);

    // stbtt_InitFont(font_info->info, ttf_buffer, stbtt_GetFontOffsetForIndex(ttf_buffer, 0));

   //fread(ttf_buffer, 1, 1<<20, fopen("c:/windows/fonts/times.ttf", "rb"));
   //stbtt_BakeFontBitmap(ttf_buffer, 0, 20.0, temp_bitmap,
   //        512, 512, 32, 96, cdata); // no guarantee this fits!
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
    SDL_SetTextureColorMod(font_data->atlas, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(font_data->atlas, color.a);
    for(int i = 0; text[i]; i++) {
        // Check char is within ascii range
        if ((int)text[i] >= 32 && (int)text[i] < 128) {
            stbtt_packedchar* info = &font_data->chars[text[i] - 32];
            SDL_Rect src_rect = {info->x0, info->y0, info->x1 - info->x0, info->y1 - info->y0};
            SDL_Rect dst_rect = {pos.x + info->xoff, pos.y + info->yoff, info->x1 - info->x0, info->y1 - info->y0};

            SDL_RenderCopy(renderer, font_data->atlas, &src_rect, &dst_rect);
            pos.x += info->xadvance;
        }
    }






























    //SDL_Texture * texture = SDL_CreateTexture(
    //    renderer,
    //    //SDL_PIXELFORMAT_ARGB8888,
    //    //SDL_PIXELFORMAT_RGBA8888,
    //    SDL_PIXELFORMAT_RGBA32,
    //    SDL_TEXTUREACCESS_STATIC,
    //    cdata[10].x1-cdata[10].x0,
    //    cdata[10].y1-cdata[10].y0
    //);
    //SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
    //SDL_PixelFormat* format = SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32);

    //Uint32* pixels = malloc(10000 * sizeof(Uint32));
    //for(int i = 0; i < 10000; i++) {
    //    pixels[i] = SDL_MapRGBA(format, 0xff, 0xff, 0xff, temp_bitmap[i]);
    //}

    //SDL_UpdateTexture(texture, NULL, pixels, 10000 * sizeof(Uint32));
    ////SDL_UpdateTexture(texture, NULL, temp_bitmap, cdata[10].y1-cdata[10].y0);
    //SDL_RenderCopy(renderer, texture, NULL, NULL);
    ////SDL_RenderPresent(renderer);





    // FontInfo* font = calloc(sizeof(FontInfo), 1);
    // font->info = malloc(sizeof(stbtt_fontinfo));
    // font->chars = malloc(sizeof(stbtt_packedchar) * 96);

    // int index = text[0] - 32;

    // SDL_Rect src;
    // src.x = cdata[index].x0;
    // src.y = cdata[index].y0;
    // src.w = cdata[index].x1 - cdata[index].x0;
    // src.h = cdata[index].y1 - cdata[index].y0;

    // SDL_Rect dest;
    // src.x = pos.x;
    // src.y = pos.y;
    // src.w = 20;
    // src.h = 20;

    // SDL_Texture * texture = SDL_CreateTexture(
    //     renderer,
    //     SDL_PIXELFORMAT_RGBA32,
    //     SDL_TEXTUREACCESS_STATIC,
    //     40,
    //     40
    // );
    // SDL_Rect rect;
    // rect.x = pos.x;
    // rect.y = pos.y;
    // rect.w = 20;
    // rect.h = 20;
    // SDL_RenderCopy(renderer, texture, NULL, &rect);







    // SDL_Rect rect;
    // rect.x = pos.x;
    // rect.y = pos.y;
    // rect.w = 20;
    // rect.h = 20;
    // SDL_RenderFillRect(renderer, &rect);



/*
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
*/





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
