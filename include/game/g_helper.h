#ifndef G_HELPER_H
#define G_HELPER_H

#include "g_map_types.h"
#include <stdbool.h>

#include <sdl/SDL.h>

//retuns true if the informed position is off the grid
bool g_is_off_grid(int grid_x, int grid_y);
//convert mouse position(window) into grid position, returns false if off grid
bool g_mouse_to_grid(int mouse_x, int mouse_y, int* grid_x, int* grid_y);
SDL_Rect g_get_tile_rect(int grid_x, int grid_y);
//retruns -1 if off grid
int g_grid_to_index(int grid_x, int grid_y);

#endif//G_HELPER_H