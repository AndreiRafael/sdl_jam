#include <game/g_scenes.h>
#include <math.h>
#include <stdio.h>

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

    int x, y;
    SDL_GetMouseState(&x, &y);
    int tex_x, tex_y;
    hf_scene_window_pos_to_render_pos(x, y, &tex_x, &tex_y);
    int win_x, win_y;
    hf_scene_render_pos_to_window_pos(tex_x, tex_y, &win_x, &win_y);
    printf("window: { %d/%d, %d/%d } -> texture: { %d, %d }\n", x, win_x, y, win_y, tex_x, tex_y);
}

static void render(SDL_Renderer* renderer) {
    Uint8 val = (Uint8)((sinf(value) + 1.f) * 0.5f * 255.f * 0.5f);
    SDL_SetRenderDrawColor(renderer, val, val, val, 255);
    SDL_RenderClear(renderer);
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