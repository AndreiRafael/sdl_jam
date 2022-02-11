#include <game/g_scenes.h>
#include <game/g_helper.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <sdl/SDL_image.h>

static SDL_Texture* ground_texture;
static SDL_Texture* entities_texture;

static g_ground_type* tiles = NULL;
static g_ground_type selected_ground_type;
static g_entity_type selected_entity_type;
static g_entity_type* entities = NULL;

static bool placing_ground = true;//can only be placing ground or entities
static float grid_flash = 0.f;

static struct {
    SDL_Rect rect;
    g_ground_type type;
} ground_buttons[g_ground_type_max_value];

static struct {
    SDL_Rect rect;
    g_entity_type type;
} entity_buttons[g_entity_type_max_value];

static void save_map() {
    FILE* file = fopen("./maps/editor.map", "wb");

    int tile_count = g_map_size * g_map_size;
    for(int i = 0; i < tile_count; i++) {//write tiles
        unsigned char val = (unsigned char)tiles[i];
        fwrite(&val, sizeof(unsigned char), 1, file);
    }
    for(int i = 0; i < tile_count; i++) {//write entities
        unsigned char val = (unsigned char)entities[i];
        fwrite(&val, sizeof(unsigned char), 1, file);
    }

    fclose(file);
}

static void load_map() {
    FILE* file = fopen("./maps/editor.map", "rb");
    if(!file) {
        return;
    }

    int tile_count = g_map_size * g_map_size;
    for(int i = 0; i < tile_count; i++) {//read tiles
        unsigned char val;
        fread(&val, sizeof(unsigned char), 1, file);
        tiles[i] = (g_ground_type)val;
    }
    for(int i = 0; i < tile_count; i++) {//read entities
        unsigned char val;
        fread(&val, sizeof(unsigned char), 1, file);
        entities[i] = (g_entity_type)val;
    }

    fclose(file);
}

static SDL_Rect get_ground_texture_rect(g_ground_type type) {
    int h;
    SDL_QueryTexture(ground_texture, NULL, NULL, NULL, &h);

    return (SDL_Rect){
        .x = (int)type * h,
        .y = 0,
        .w = h,
        .h = h
    };
}

static SDL_Rect get_entity_texture_rect(g_entity_type type) {
    int h;
    SDL_QueryTexture(entities_texture, NULL, NULL, NULL, &h);

    return (SDL_Rect){
        .x = ((int)type - 1) * h,
        .y = 0,
        .w = h,
        .h = h
    };
}

static void draw_map(SDL_Renderer* renderer) {
    for(int x = 0; x < g_map_size; x++) {
        for(int y = 0; y < g_map_size; y++) {
            const int index = g_grid_to_index(x, y);
            const g_ground_type ground_type = tiles[index];
            SDL_Rect ground_src_rect = get_ground_texture_rect(ground_type);

            SDL_Rect tile_rect = g_get_tile_rect(x, y);
            SDL_RenderCopy(renderer, ground_texture, &ground_src_rect, &tile_rect);

            const g_entity_type entity_type = entities[index];
            if(entity_type != g_entity_type_none) {
                const SDL_Rect entity_src_rect = get_entity_texture_rect(entity_type);

                SDL_Rect entity_rect = {
                    .x = tile_rect.x + tile_rect.w / 4,
                    .y = tile_rect.y + tile_rect.h / 4,
                    .w = tile_rect.w / 2,
                    .h = tile_rect.h / 2
                };
                SDL_RenderCopy(renderer, entities_texture, &entity_src_rect, &entity_rect);
            }
        }
    }
}

//generic scene functions
static void init(void) {
    int win_w, win_h;
    hf_scene_get_render_size(&win_w, &win_h);

    int tile_count = g_map_size * g_map_size;
    tiles = calloc((size_t)tile_count, sizeof(g_ground_type));
    for(int i = 0; i < tile_count; i++) {
        tiles[i] = g_ground_type_dirt;
    }

    entities = calloc((size_t)tile_count, sizeof(g_entity_type));
    for(int i = 0; i < tile_count; i++) {
        entities[i] = g_entity_type_none;
    }

    placing_ground = true;
    selected_ground_type = g_ground_type_dirt;

    //setup side tool buttons
    const int button_x_start = g_map_size * g_tile_size;
    const int buttons_per_row = 2;
    const int button_padding = 10;
    const int button_size = 40;
    //setup ground buttons
    for(int i = 0; i < g_ground_type_max_value; i++) {
        const int row = i / buttons_per_row;
        const int x_index = i - row * buttons_per_row;

        ground_buttons[i].rect = (SDL_Rect){
            .x = button_x_start + button_padding * (x_index + 1) + button_size * x_index,
            .y = button_padding * (row + 1) + button_size * row,
            .w = button_size,
            .h = button_size
        };
        ground_buttons[i].type = (g_ground_type)i;
    }

    //setup entity buttons
    const int ground_rows = g_ground_type_max_value / buttons_per_row;
    for(int i = 0; i < g_entity_type_max_value; i++) {
        const int row = i / buttons_per_row;
        const int x_index = i - row * buttons_per_row;
        const int row_offset = ground_rows + 1;

        entity_buttons[i].rect = (SDL_Rect){
            .x = button_x_start + button_padding * (x_index + 1) + button_size * x_index,
            .y = button_padding * (row + row_offset + 1) + button_size * (row + row_offset),
            .w = button_size,
            .h = button_size
        };
        entity_buttons[i].type = (g_entity_type)i;
    }

    SDL_Renderer* renderer = hf_scene_get_renderer();
    ground_texture = IMG_LoadTexture(renderer, "./res/ground.png");
    entities_texture = IMG_LoadTexture(renderer, "./res/entities.png");
}

static void quit(void) {
    free(tiles);
    free(entities);

    SDL_DestroyTexture(ground_texture);
    SDL_DestroyTexture(entities_texture);
}

static void process_input(SDL_Event e) {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    int tex_x, tex_y;
    hf_scene_window_pos_to_render_pos(mouse_x, mouse_y, &tex_x, &tex_y);

    switch(e.type) {
        case SDL_KEYDOWN:
            if(e.key.keysym.sym == SDLK_s){
                save_map();
            }
            else if(e.key.keysym.sym == SDLK_l) {
                load_map();
            }
            break;
        case SDL_MOUSEBUTTONDOWN:
            if(e.button.button == SDL_BUTTON_LEFT) {//left click
                //check clicks on ground buttons
                for(int i = 0; i < g_ground_type_max_value; i++) {
                    SDL_Point p = { .x = tex_x, .y = tex_y };
                    if(SDL_PointInRect(&p, &ground_buttons[i].rect)) {
                        selected_ground_type = ground_buttons[i].type;
                        placing_ground = true;
                    }
                }

                //check clicks on entity buttons
                for(int i = 0; i < g_entity_type_max_value; i++) {
                    SDL_Point p = { .x = tex_x, .y = tex_y };
                    if(SDL_PointInRect(&p, &entity_buttons[i].rect)) {
                        selected_entity_type = entity_buttons[i].type;
                        placing_ground = false;
                    }
                }
            }
            break;
    }
}

static void update(float delta_time) {
    grid_flash += delta_time * 5.f;

    int mouse_x, mouse_y;
    Uint32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
    int grid_x, grid_y;
    bool grid_valid = g_mouse_to_grid(mouse_x, mouse_y, &grid_x, &grid_y);    

    if(grid_valid) {
        int tile_index = g_grid_to_index(grid_x, grid_y);
        if(placing_ground) {
            if(mouse_buttons & SDL_BUTTON_LMASK) {//holding left mouse button
                tiles[tile_index] = selected_ground_type;
            }
        }
        else {
            if(mouse_buttons & SDL_BUTTON_LMASK) {//holding left mouse button
                entities[tile_index] = selected_entity_type;
            }
        }
    }    
}

static void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 50, 50, 50, 255);
    SDL_RenderClear(renderer);

    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);

    int tex_x, tex_y;
    hf_scene_window_pos_to_render_pos(mouse_x, mouse_y, &tex_x,  &tex_y);

    //draw ground buttons
    for(int i = 0; i < g_ground_type_max_value; i++) {
        g_ground_type t = ground_buttons[i].type;
        SDL_Rect src_rect = get_ground_texture_rect(t);

        SDL_RenderCopy(renderer, ground_texture, &src_rect, &ground_buttons[i].rect);
        if(placing_ground && t == selected_ground_type) {//draw selected highlight
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderDrawRect(renderer, &ground_buttons[i].rect);

            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100);
            SDL_RenderDrawRect(renderer, &ground_buttons[i].rect);            
        }
        SDL_Point p = { .x = tex_x, .y = tex_y };
        if(SDL_PointInRect(&p, &ground_buttons[i].rect)) {// mouse hover
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100);
            SDL_RenderFillRect(renderer, &ground_buttons[i].rect);   
        }
    }

    //draw entity buttons
    for(int i = 0; i < g_entity_type_max_value; i++) {
        g_entity_type t = entity_buttons[i].type;
        SDL_Rect src_rect = get_entity_texture_rect(t);

        SDL_RenderCopy(renderer, entities_texture, &src_rect, &entity_buttons[i].rect);
        if(!placing_ground && t == selected_entity_type) {//draw selected highlight
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 255);
            SDL_RenderDrawRect(renderer, &entity_buttons[i].rect);

            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100);
            SDL_RenderDrawRect(renderer, &entity_buttons[i].rect);            
        }
        SDL_Point p = { .x = tex_x, .y = tex_y };
        if(SDL_PointInRect(&p, &entity_buttons[i].rect)) {// mouse hover
            SDL_SetRenderDrawColor(renderer, 0, 0, 255, 100);
            SDL_RenderFillRect(renderer, &entity_buttons[i].rect);   
        }
    }
    

    draw_map(renderer);
    int grid_x, grid_y;

    //draw grid flash over everything
    if(g_mouse_to_grid(mouse_x, mouse_y, &grid_x, &grid_y)) {
        SDL_Rect r = g_get_tile_rect(grid_x, grid_y);
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
        SDL_RenderDrawRect(renderer, &r);
        Uint8 alpha = (Uint8)(fabsf(sinf(grid_flash)) * 100.f) + 20u;
        SDL_SetRenderDrawColor(renderer, 255, 255, 255, alpha);
        SDL_RenderFillRect(renderer, &r);
    }
}

hf_scene  g_scene_get_editor(void) {
    hf_scene new_scene = {
        .init = init,
        .quit = quit,
        .process_input = process_input,
        .update = update,
        .render = render
    };
    return new_scene;
}