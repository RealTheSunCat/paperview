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

SDL_Rect* rectangle;

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
int scaleRes(int val, int isY)
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

    SDL_Rect* rect = malloc(sizeof(*rect));
    rect->x = scaleRes(760, False);
    rect->w = scaleRes(400, False);
    rect->h = scaleRes(400, True);
    cacheTextures(argv[1], argv[2], video->renderer);
    rectangle = rect;
}

int main(int argc, char** argv)
{
    Video video = setup();
    parseArgs(argc, argv, &video);
    for(int cycles = 0; /* true */; cycles++)
    {
        // render
        SDL_RenderCopy(video.renderer, background, NULL, NULL);

        rectangle->y = scaleRes((int) (440 + sin((double)cycles / 200.0f) * 50.0f), True); // oscillate slowly between 390 and 490
        SDL_RenderCopy(video.renderer, sprite, NULL, rectangle);

        SDL_RenderPresent(video.renderer);
        SDL_Event event;
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT)
            break;
    }

    cleanup();
    teardown(&video);
}
