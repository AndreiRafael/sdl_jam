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
void hf_scene_get_render_size(int* width, int* height);
void hf_scene_window_pos_to_render_pos(int window_x, int window_y, int* render_x, int* render_y);
void hf_scene_render_pos_to_window_pos(int render_x, int render_y, int* window_x, int* window_y);
void hf_scene_load(hf_scene scene);
bool hf_scene_update(void);
void hf_scene_request_quit(void);

#endif