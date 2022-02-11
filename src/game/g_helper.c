#include <game/g_helper.h>
#include <hf/hf_scene.h>

//retuns true if the informed position is off the grid
bool g_is_off_grid(int grid_x, int grid_y) {
    return grid_x < 0 || grid_y < 0 || grid_x >= g_map_size || grid_y >= g_map_size;
}

//convert mouse position(window) into grid position, returns false if off grid
bool g_mouse_to_grid(int mouse_x, int mouse_y, int* grid_x, int* grid_y) {
    int tex_x, tex_y;    
    hf_scene_window_pos_to_render_pos(mouse_x, mouse_y, &tex_x, &tex_y);

    int g_x, g_y;
    g_x = tex_x / g_tile_size;
    g_y = tex_y / g_tile_size;
    if(grid_x) {
        *grid_x = g_x;
    }
    if(grid_y) {
        *grid_y = g_y;
    }

    return !g_is_off_grid(g_x, g_y);
}

SDL_Rect g_get_tile_rect(int grid_x, int grid_y) {
    if(g_is_off_grid(grid_x, grid_y)) {
        return (SDL_Rect){ 0, 0, 0, 0 };
    }

    return (SDL_Rect){
        .x = grid_x * g_tile_size,
        .y = grid_y * g_tile_size,
        g_tile_size,
        g_tile_size
    };
}

//retruns -1 if off grid
int g_grid_to_index(int grid_x, int grid_y) {
    if(g_is_off_grid(grid_x, grid_y)) {
        return -1;
    }

    return grid_x + grid_y * g_map_size;
}