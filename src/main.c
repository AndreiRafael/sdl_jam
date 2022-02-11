#if (defined(__MINGW32__) || defined(__GNUC__)) && defined(_WIN32)
#define SDL_MAIN_HANDLED
#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS 1
#endif//_MSC_VER

#include <sdl/SDL.h>
#include <sdl/SDL_image.h>
#include <stdio.h>
#include <stdbool.h>

#include <hf/hf_scene.h>
#include <hf/hf_dir.h>

#include <game/g_scenes.h>

int main(int argc, char* argv[]) {
#if defined(SDL_MAIN_HANDLED)
    SDL_SetMainReady();
#endif
    //warning suppression
    (void)argc;
    (void)argv;

    SDL_Init(SDL_INIT_VIDEO);
    IMG_Init(IMG_INIT_PNG);

    SDL_Window* window = SDL_CreateWindow(
        "sdl_jam",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        1000, 800,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE
    );

    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);

    hf_dir_entry* entry = hf_dir_entry_create(SDL_GetBasePath());
    if(entry) {
        printf("path: %s\n\n", entry->path);
    }
    size_t count;
    hf_dir_entry** entry2 = hf_dir_entries_create("D:/Andrei", &count);
    if(entry2) {
        for(size_t i = 0; i < count; i++) {
            if(entry2[i]) {
                printf("path[%d]: %s", (int)i, entry2[i]->path);
                if(entry2[i]->type == hf_dir_entry_file) {
                    printf(" - file\n");
                }
                else {
                    printf(" - folder\n");
                }
            }
        }
    }

    hf_scene_setup(renderer);
    hf_scene_set_render_size(1000, 800);
    hf_scene_load(g_scene_get_game());
    while(hf_scene_update());
    
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();

    return 0;
}