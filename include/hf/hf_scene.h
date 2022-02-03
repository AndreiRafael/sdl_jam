#ifndef HF_SCENE_H
#define HF_SCENE_H

#include <sdl/SDL.h>
#include <stdbool.h>

typedef struct hf_scene_t {
    void (*init)(void);
    void (*quit)(void);

    void (*process_input)(SDL_Event event);
    void (*update)(float delta_time);
    void (*render)(SDL_Renderer* renderer);
} hf_scene;

void hf_scene_setup(SDL_Renderer* renderer);
void hf_scene_set_render_size(int width, int height);
void hf_scene_load(hf_scene scene);
bool hf_scene_update(void);
void hf_scene_request_quit(void);

#endif