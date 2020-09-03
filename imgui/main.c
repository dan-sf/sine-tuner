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
void render()
{   
  // clear screen
    Color color;
    color.r = 50; color.b = 50; color.g = 50; color.a = 0xff;
  drawrect(0,0,640,480,color);

  // test that the fillrect is working
    color.r = 230; color.b = 230; color.g = 230;
  drawrect(64,48,64,48,color);

  // update the screen
    SDL_RenderPresent(renderer);

  // don't take all the cpu time
  SDL_Delay(10); 
}


// Entry point
int main(int argc, char *argv[])
{
  // Initialize SDL's subsystems - in this case, only video.
    if (SDL_Init(SDL_INIT_VIDEO) < 0) 
    {
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
    while (1)
    {
    // Render stuff
        render();

    // Poll for events, and handle the ones we care about.
        SDL_Event event;
        while (SDL_PollEvent(&event)) 
        {
            switch (event.type) 
            {
            case SDL_KEYUP:
                    
        switch (event.key.keysym.sym)
        {
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
