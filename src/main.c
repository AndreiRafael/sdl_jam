#if (defined(__MINGW32__) || defined(__GNUC__)) && defined(_WIN32)
#define SDL_MAIN_HANDLED
#endif

#include <sdl/SDL.h>
#include <stdio.h>
#include <stdbool.h>

#include <hf/hf_scene.h>

#include <game/g_scenes.h>

int main(int argc, char* argv[]) {
#if defined(SDL_MAIN_HANDLED)
    SDL_SetMainReady();
#endif
    //warning suppression
    (void)argc;
    (void)argv;

    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window* window = SDL_CreateWindow(
        "sdl_jam",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    hf_scene_setup(renderer);
    hf_scene_load(g_scene_get_menu());
    while(hf_scene_update());
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}