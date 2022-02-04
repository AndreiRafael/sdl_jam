#include <hf/hf_scene.h>

static bool is_loaded = false;
static bool is_queued = false;
static bool quit_requested = false;
static hf_scene queued_scene;
static hf_scene current_scene;

static SDL_Renderer* sdl_renderer = NULL;
static Uint32 prev_ticks = 0u;
static SDL_Texture* render_texture = NULL;

void hf_scene_setup(SDL_Renderer* renderer) {
    sdl_renderer = renderer;
}

void hf_scene_set_render_size(int width, int height) {
    if(render_texture) {
        SDL_DestroyTexture(render_texture);
        render_texture = NULL;
    }

    if(width > 0 && height > 0) {
        render_texture = SDL_CreateTexture(
            sdl_renderer, SDL_PIXELFORMAT_RGBA32,
            SDL_TEXTUREACCESS_TARGET,
            width, height
        );
    }
}

void hf_scene_get_render_size(int* width, int* height) {
    if(render_texture) {
        SDL_QueryTexture(render_texture, NULL, NULL, width, height);
    }
    else {
        SDL_Texture* prev_target = SDL_GetRenderTarget(sdl_renderer);
        SDL_SetRenderTarget(sdl_renderer, NULL);
        SDL_GetRendererOutputSize(sdl_renderer, width, height);
        SDL_SetRenderTarget(sdl_renderer, prev_target);
    }
}

static void calculate_render_scale(SDL_Rect* render_rect, float* render_scale) {
    SDL_Texture* prev_target = SDL_GetRenderTarget(sdl_renderer);
    SDL_SetRenderTarget(sdl_renderer, NULL);

    int r_width, r_height;//renderer dimensions
    SDL_GetRendererOutputSize(sdl_renderer, &r_width, &r_height);
    int t_width, t_height;//texture dimensions
    hf_scene_get_render_size(&t_width, &t_height);
    float scale = SDL_min((float)r_width / (float)t_width, (float)r_height / (float)t_height);

    if(render_rect) {
        *render_rect = (SDL_Rect){
            .x = (int)(0.5f * ((float)r_width - (float)t_width * scale)),
            .y = (int)(0.5f * ((float)r_height - (float)t_height * scale)),
            .w = (int)((float)t_width * scale),
            .h = (int)((float)t_height * scale)
        };
    }
    if(render_scale) {
        *render_scale = scale;
    }

    SDL_SetRenderTarget(sdl_renderer, prev_target);
}

void hf_scene_window_pos_to_render_pos(int window_x, int window_y, int* render_x, int* render_y) {
    float scale;
    SDL_Rect rect;
    calculate_render_scale(&rect, &scale);

    if(render_x) {
        *render_x = (int)SDL_lroundf((float)(window_x - rect.x) / scale);
    }
    if(render_y) {
        *render_y = (int)SDL_lroundf((float)(window_y - rect.y) / scale);
    }
}

void hf_scene_render_pos_to_window_pos(int render_x, int render_y, int* window_x, int* window_y) {
    float scale;
    SDL_Rect rect;
    calculate_render_scale(&rect, &scale);

    if(window_x) {
        *window_x = (int)SDL_lroundf((float)(render_x) * scale) + rect.x;
    }
    if(window_y) {
        *window_y = (int)SDL_lroundf((float)(render_y) * scale) + rect.y;
    }
}


void hf_scene_load(hf_scene scene) {
    is_queued = true;
    queued_scene = scene;
}

bool hf_scene_update(void) {
    if(!sdl_renderer) {
        return false;
    }

    if(is_queued) {
        if(is_loaded) {
            current_scene.quit();
        }
        current_scene = queued_scene;
        current_scene.init();

        prev_ticks = SDL_GetTicks();
        is_loaded = true;
        is_queued = false;
    }

    bool quit = false;
    if(is_loaded) {
        //EVENT HANDLING
        SDL_Event e;
        while(SDL_PollEvent(&e)) {
            switch (e.type)
            {
            case SDL_QUIT:
                quit = true;
                break;            
            default:
                current_scene.process_input(e);
                break;
            }
        }

        // TIMER & UPDATE
        Uint32 new_ticks = SDL_GetTicks();
        Uint32 delta_ticks = new_ticks - prev_ticks;
        current_scene.update((float)delta_ticks / 1000.f);
        prev_ticks = new_ticks;

        // RENDER
        if(render_texture) {//only clear renderer if we have a render texture
            SDL_SetRenderDrawColor(sdl_renderer, 0, 0, 0, 255);
            SDL_SetRenderTarget(sdl_renderer, NULL);
            SDL_RenderClear(sdl_renderer);
            SDL_SetRenderTarget(sdl_renderer, render_texture);
        }
        current_scene.render(sdl_renderer);

        if(render_texture) {//copy texture to main renderer, scaled
            SDL_Rect dest_rect;
            calculate_render_scale(&dest_rect, NULL);
            SDL_SetRenderTarget(sdl_renderer, NULL);
            SDL_RenderCopy(sdl_renderer, render_texture, NULL, &dest_rect);
        }
        SDL_RenderPresent(sdl_renderer);

        if(quit || quit_requested) {//properly quit current scene
            current_scene.quit();
        }
    }

    bool ret_val = !quit && !quit_requested;
    if(!ret_val) {// reset variables for eventual new loop
        is_loaded = is_queued = quit_requested = false;
        if(render_texture) {
            //destroy render texture, new instances must create their own
            SDL_DestroyTexture(render_texture);
            render_texture = NULL;
        }
        sdl_renderer = NULL;
    }
    return ret_val;
}

void hf_scene_request_quit(void) {
    quit_requested = true;
}