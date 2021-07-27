#include <SDL2/SDL.h>
#include <X11/Xlib.h>
#include <dirent.h>
#include <stdarg.h>
#include <math.h>

const int xRes = 1920; const int yRes = 1080;

typedef struct
{
    Display* x11d;
    SDL_Window* window;
    SDL_Renderer* renderer;
}
Video;

SDL_Texture* background;
SDL_Texture* sprite;

SDL_FRect* rectangle;

static void quit(const char* const message, ...)
{
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
    exit(1);
}

static void cacheTextures(const char* bgPath, const char* spritePath, SDL_Renderer* renderer)
{
    // this should be a func but ah well
    {
        SDL_Surface* const surface = SDL_LoadBMP(bgPath);
        if(surface == NULL)
            quit("Background file failed to open: %s\n", SDL_GetError());
        background = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    {
        SDL_Surface* const surface = SDL_LoadBMP(spritePath);
        if(surface == NULL)
            quit("Sprite file failed to open: %s\n", SDL_GetError());
        sprite = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
}

static Video setup(void)
{
    Video video;
    video.x11d = XOpenDisplay(NULL);
    const Window x11w = RootWindow(video.x11d, DefaultScreen(video.x11d));
    SDL_Init(SDL_INIT_VIDEO);
    video.window = SDL_CreateWindowFrom((void*) x11w);
    video.renderer = SDL_CreateRenderer(video.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    return video;
}

static void teardown(Video* self)
{
    XCloseDisplay(self->x11d);
    SDL_Quit();
    SDL_DestroyWindow(self->window);
    SDL_DestroyRenderer(self->renderer);
}

static void cleanup()
{
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(sprite);
    free(rectangle);
}

// ugly code but I'm a C++ programmer, not C
float scaleRes(float val, int isY)
{
    float defaultRes = 1920;
    float actualRes = xRes;
    if(isY)
    {
        defaultRes = 1080;
        actualRes = yRes;
    }

    return ((float) val) * (((float) actualRes) / defaultRes);
}

static void parseArgs(int argc, char** argv, Video* video)
{
    if(argc < 3)
        quit("Usage: paperview background.bmp sprite.bmp\n");

    SDL_FRect* rect = malloc(sizeof(*rect));
    rect->x = scaleRes(710.0f, False);
    rect->w = scaleRes(500.0f, False);
    rect->h = scaleRes(500.0f, True);
    cacheTextures(argv[1], argv[2], video->renderer);
    rectangle = rect;
}

float ease(float time, float startValue, float change, float duration) {
     time /= duration / 2;
     if (time < 1)  {
          return change / 2 * time * time + startValue;
     }

     time--;
     return -change / 2 * (time * (time - 2) - 1) + startValue;
 }


int main(int argc, char** argv)
{
    Video video = setup();
    parseArgs(argc, argv, &video);

    int animLength = 300; // 5s at 60fps
    for(int cycles = 0; /* true */; cycles++)
    {
        // render
        SDL_RenderCopy(video.renderer, background, NULL, NULL);

        // emulate CSS anim
        int animTime = cycles % animLength;

        if(animTime >= 0 && animTime < 0.5*animLength)
        {
            rectangle->y = ease(animTime, 0, -20, 0.5*animLength);
        } else { // animTime > 0.5*animLength
            rectangle->y = ease(animTime - 0.5*animLength, -20, 20, 0.5*animLength);
        }

        rectangle->y += 540 - 250; // center
        rectangle->y = scaleRes(rectangle->y, True); // scale for screen res

        SDL_RenderCopyF(video.renderer, sprite, NULL, rectangle);
        
        SDL_RenderPresent(video.renderer);
        SDL_Event event;
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT)
            break;
    }

    cleanup();
    teardown(&video);
}
