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

        Uint32 new_ticks = SDL_GetTicks();
        Uint32 delta_ticks = new_ticks - prev_ticks;
        current_scene.update((float)delta_ticks / 1000.f);
        prev_ticks = new_ticks;

        // TODO: set render target and strech texture to screen
        current_scene.render(sdl_renderer);
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
        }
        sdl_renderer = NULL;
    }
    return ret_val;
}

void hf_scene_request_quit(void) {
    quit_requested = true;
}