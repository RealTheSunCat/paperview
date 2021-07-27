#include <SDL2/SDL.h>
#include <X11/Xlib.h>
#include <dirent.h>
#include <stdarg.h>

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

static void Quit(const char* const message, ...)
{
    va_list args;
    va_start(args, message);
    vfprintf(stdout, message, args);
    va_end(args);
    exit(1);
}

static void CacheTextures(const char* bgPath, const char* spritePath, SDL_Renderer* renderer)
{
    {
        SDL_Surface* const surface = SDL_LoadBMP(bgPath);
        if(surface == NULL)
            Quit("Background file failed to open: %s\n", SDL_GetError());
        background = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
    {
        SDL_Surface* const surface = SDL_LoadBMP(spritePath);
        if(surface == NULL)
            Quit("Sprite file failed to open: %s\n", SDL_GetError());
        sprite = SDL_CreateTextureFromSurface(renderer, surface);
        SDL_FreeSurface(surface);
    }
}

static void DestroyTextures()
{
    SDL_DestroyTexture(background);
    SDL_DestroyTexture(sprite);
}

static Video Setup(void)
{
    Video video;
    video.x11d = XOpenDisplay(NULL);
    const Window x11w = RootWindow(video.x11d, DefaultScreen(video.x11d));
    SDL_Init(SDL_INIT_VIDEO);
    video.window = SDL_CreateWindowFrom((void*) x11w);
    video.renderer = SDL_CreateRenderer(video.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    return video;
}

static void Teardown(Video* self)
{
    XCloseDisplay(self->x11d);
    SDL_Quit();
    SDL_DestroyWindow(self->window);
    SDL_DestroyRenderer(self->renderer);
}

static View* Init(const char* const base, const int speed, SDL_Rect* rect, Video* video)
{
    CacheTextures(&paths, video->renderer);
    rectangle = rect;
    return self;
}

static View* Push(View* views, View* view)
{
    view->next = views;
    return view;
}

static void Cleanup(View* views)
{
    View* view = views;
    while(view)
    {
        View* next = view->next;
        Destroy(&view->textures);
        free(view->rect);
        free(view);
        view = next;
    }
}

static View* Parse(int argc, char** argv, Video* video)
{
    const int args = argc - 1;
    if(args < 2)
        Quit("Usage: paperview FOLDER SPEED\n"); // LEGACY PARAMETER SUPPORT.
    const int params = 6;
    if(args > 2 && args % params != 0)
        Quit("Usage: paperview FOLDER SPEED X Y W H FOLDER SPEED X Y W H # ... And so on\n"); // MULTI-MONITOR PARAMETER SUPPORT.
    View* views = NULL;
    for(int i = 1; i < argc; i += params)
    {
        const int a = i + 0;
        const int b = i + 1;
        const int c = i + 2;
        const int d = i + 3;
        const int e = i + 4;
        const int f = i + 5;
        const char* const base = argv[a];
        int speed = atoi(argv[b]);
        if(speed == 0)
            Quit("Invalid speed value\n");
        if(speed < 0)
            speed = INT32_MAX; // NEGATIVE SPEED VALUES CREATE STILL WALLPAPERS.
        SDL_Rect* rect = NULL;
        if(c != argc)
        {
            rect = malloc(sizeof(*rect));
            rect->x = atoi(argv[c]);
            rect->y = atoi(argv[d]);
            rect->w = atoi(argv[e]);
            rect->h = atoi(argv[f]);
        }
        views = Push(views, Init(base, speed, rect, video));
    }
    return views;
}

int main(int argc, char** argv)
{
    Video video = Setup();
    View* views = Parse(argc, argv, &video);
    for(int cycles = 0; /* true */; cycles++)
    {
        for(View* view = views; view; view = view->next)
        {
            const int index = cycles / view->speed;
            const int frame = index % view->textures.size;
            SDL_RenderCopy(video.renderer, view->textures.texture[frame], NULL, view->rect);
        }
        SDL_RenderPresent(video.renderer);
        SDL_Event event;
        SDL_PollEvent(&event);
        if(event.type == SDL_QUIT)
            break;
    }
    Cleanup(views);
    Teardown(&video);
}
