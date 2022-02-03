#include <game/g_scenes.h>
#include <math.h>

static float value = 0.f;

static void init(void) {
    value = 0.f;
}

static void quit(void) {

}

static void process_input(SDL_Event e) {
    (void)e;
}

static void update(float delta_time) {
    value += delta_time * 4.f;
}

static void render(SDL_Renderer* renderer) {
    Uint8 val = (Uint8)((sinf(value) + 1.f) * 0.5f * 255.f * 0.5f);
    SDL_SetRenderDrawColor(renderer, val, val, val, 255);
}

hf_scene  g_scene_get_menu(void) {
    hf_scene new_scene = {
        .init = init,
        .quit = quit,
        .process_input = process_input,
        .update = update,
        .render = render
    };
    return new_scene;
}