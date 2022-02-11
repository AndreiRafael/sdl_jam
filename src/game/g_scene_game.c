#include <game/g_scenes.h>
#include <game/g_helper.h>
#include <hf/hf_vec2.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <sdl/SDL_image.h>

static SDL_Texture* ground_texture;
static SDL_Texture* entities_texture;
static SDL_Texture* buttons_texture;

enum {
    button_count = 3,
    button_id_daycycle = 0,
    button_id_restart = 1,
    button_id_home = 2,
};

enum {
    progress_bar_padding = 10,
    progress_bar_height = 20
};
static struct {
    SDL_Rect src_rect;
    SDL_Rect dest_rect;
    void (*on_click)(void);
} buttons[button_count];

static g_ground_type tiles[g_map_size * g_map_size];
static struct entity_t {
    g_entity_type type;
    hf_vec2 pos;
    hf_vec2 dir;
} *entities;
static size_t entity_count;

static bool is_night = false;

static bool get_rain(hf_vec2* rain_pos) {
    int mouse_x, mouse_y;
    int tex_x, tex_y;
    Uint32 mouse_buttons = SDL_GetMouseState(&mouse_x, &mouse_y);
    hf_scene_window_pos_to_render_pos(mouse_x, mouse_y, &tex_x, &tex_y);

    if(rain_pos) {
        *rain_pos = (hf_vec2){ .x = (float)tex_x / (float)g_tile_size, .y = (float)tex_y / (float)g_tile_size };
    }

    return mouse_buttons & SDL_BUTTON_LMASK;
}

//speed of entity when too close to others
static float entity_runaway_speed(g_entity_type type) {
    switch(type) {
        case g_entity_type_white_shepherd:
        case g_entity_type_black_shepherd:
        case g_entity_type_white_flag:
        case g_entity_type_black_flag:
            return 0.f;
        default:
            return 1.f;
    }
}

//speed of entity when normally walkin
static float entity_normal_speed(g_entity_type type) {
    switch(type) {
        case g_entity_type_wolf:
            return 0.9f;
        default:
            return 1.f;
    }
}

//finds closest entity in range, use type == none to get any entity
static bool find_closest_entity(const struct entity_t* searcher, g_entity_type type, float max_range, hf_vec2* out) {
    float best = max_range * max_range;
    bool res = false;

    for(size_t i = 0; i < entity_count; i++) {
        struct entity_t *e = entities + i;
        if(e == searcher) {
            continue;
        }

        if(type == g_entity_type_none || e->type == type) {
            float sqr_d = hf_vec2_sqr_distance(e->pos, searcher->pos);
            if(sqr_d < best) {
                best = sqr_d;
                res = true;
                if(out) {
                    *out = e->pos;
                }
            }
        }
    }
    return res;
}

static bool find_closest_sheep(const struct entity_t* searcher, float max_range, hf_vec2* out) {
    hf_vec2 black_pos;
    bool found_black = find_closest_entity(searcher, g_entity_type_black_sheep, max_range, &black_pos);
    hf_vec2 white_pos;
    bool found_white = find_closest_entity(searcher, g_entity_type_white_sheep, max_range, &white_pos);

    if(out) {
        float best = max_range * max_range;
        if(found_black) {
            best = hf_vec2_sqr_distance(searcher->pos, black_pos);
            *out = black_pos;
        }
        if(found_white && hf_vec2_sqr_distance(searcher->pos, white_pos) < best) {
            *out = white_pos;
        }
    }

    return found_black || found_white;
}

static bool find_closest_shepherd(const struct entity_t* searcher, float max_range, hf_vec2* out) {
    hf_vec2 black_pos;
    bool found_black = find_closest_entity(searcher, g_entity_type_black_shepherd, max_range, &black_pos);
    hf_vec2 white_pos;
    bool found_white = find_closest_entity(searcher, g_entity_type_white_shepherd, max_range, &white_pos);

    if(out) {
        float best = max_range * max_range;
        if(found_black) {
            best = hf_vec2_sqr_distance(searcher->pos, black_pos);
            *out = black_pos;
        }
        if(found_white && hf_vec2_sqr_distance(searcher->pos, white_pos) < best) {
            *out = white_pos;
        }
    }

    return found_black || found_white;
}

static bool find_closest_tile(const struct entity_t* searcher, g_ground_type type, float max_range, hf_vec2* out) {
    float best = max_range * max_range;
    bool res = false;

    for(int x = 0; x < g_map_size; x++) {
        for(int y = 0; y < g_map_size; y++) {
            const int index = g_grid_to_index(x, y);
            if(index < 0 || tiles[index] != type) {
                continue;
            }

            const hf_vec2 tile_pos = { .x = (float)x + .5f, .y = (float)y + .5f };
            const float sqr_d = hf_vec2_sqr_distance(tile_pos, searcher->pos);
            if(sqr_d < best) {
                best = sqr_d;
                res = true;
                if(out) {
                    *out = tile_pos;
                }
            }
        }
    }

    return res;
}

static hf_vec2 calculate_entity_dir(const struct entity_t* entity) {
    hf_vec2 out_dir = { .x = 0.f, .y = 0.f };

    switch(entity->type) {
        case g_entity_type_black_sheep:
        case g_entity_type_white_sheep://follow human, runaway wolf, find grass, keep close to other sheep(prefer same)
        {
            g_entity_type other_type =
                entity->type == g_entity_type_black_sheep ?
                g_entity_type_white_sheep :
                g_entity_type_black_sheep
            ;

            g_entity_type shepherd_type =
                entity->type == g_entity_type_white_sheep ?
                g_entity_type_white_shepherd :
                g_entity_type_black_shepherd
            ;

            hf_vec2 destination_pos;
            if(find_closest_entity(entity, g_entity_type_wolf, 5.f, &destination_pos)) {
                out_dir = hf_vec2_sub(entity->pos, destination_pos);
            }
            else if(
                (!is_night && find_closest_entity(entity, shepherd_type, 4.f, &destination_pos)) ||
                find_closest_tile(entity, g_ground_type_grass, 3.f, &destination_pos) ||
                find_closest_entity(entity, entity->type, 4.f, &destination_pos) ||
                find_closest_entity(entity, other_type, 4.f, &destination_pos)
            ) {//found entity
                out_dir = hf_vec2_sub(destination_pos, entity->pos);
            }
            break;
        }
        case g_entity_type_wolf:
        {
            hf_vec2 destination_pos;
            if(!is_night && find_closest_shepherd(entity, 10.f, &destination_pos)) {
                out_dir = hf_vec2_sub(entity->pos, destination_pos);
            }
            else if(find_closest_sheep(entity, 5.f, &destination_pos)) {
                out_dir = hf_vec2_sub(destination_pos, entity->pos);
            }
            break;
        }
        case g_entity_type_black_shepherd:
        case g_entity_type_white_shepherd://runaway from rain during day
        {
            hf_vec2 rain_pos;
            if(!is_night && get_rain(&rain_pos) && hf_vec2_distance(entity->pos, rain_pos) < 3.f) {
                out_dir = hf_vec2_sub(entity->pos, rain_pos);
            }
        }
        break;
        default:
            out_dir = (hf_vec2) { .x = 0.f, .0f };
            break;
    }

    hf_vec2 runaway_vec;
    if(
        entity->pos.x < 0.5f || entity->pos.x > ((float)g_map_size - .5f) ||
        entity->pos.y < 0.5f || entity->pos.y > ((float)g_map_size - .5f)
    ) {//entity is outside map, try to come to center
        hf_vec2 center = { .x = (float)g_map_size / 2.f, .y = (float)g_map_size / 2.f };
        out_dir = hf_vec2_sub(center, entity->pos);
    }
    else if(
        (entity->type == g_entity_type_black_sheep ||
        entity->type == g_entity_type_white_sheep) &&
        find_closest_entity(entity, g_entity_type_none, .5f, &runaway_vec)
    ) {// collide with other entities of same type to avoid overcrowding
        return hf_vec2_mul(hf_vec2_normalized(hf_vec2_sub(entity->pos, runaway_vec)), entity_runaway_speed(entity->type));
    }
    
    return hf_vec2_normalized(out_dir);
}

static void update_entities(float delta_time) {
    //first loop for direction calculation
    for(size_t i = 0; i < entity_count; i++) {
        struct entity_t* e = entities + i;

        const hf_vec2 new_dir = calculate_entity_dir(e);
        e->dir = hf_vec2_lerp(e->dir, new_dir, delta_time * 2.f);//sudo-lerp based on delta time
    }

    //secondary loop to apply directions
    for(size_t i = 0; i < entity_count; i++) {
        struct entity_t* e = entities + i;

        e->pos = hf_vec2_add(e->pos, hf_vec2_mul(e->dir, delta_time * entity_normal_speed(e->type)));
    }
}

static void tick_tile(int x, int y) {
    int index = g_grid_to_index(x, y);
    if(index < 0) {
        return;
    }

    g_ground_type type = tiles[index];
    hf_vec2 tile_pos = { .x = (float)x + 0.5f, .y = (float)y + 0.5f };
    switch(type) {
        case g_ground_type_dirt://make grass if mouse making i rain
        {
            hf_vec2 rain_pos;
            if(get_rain(&rain_pos) && hf_vec2_distance(tile_pos, rain_pos) < 2.f) {
                tiles[index] = g_ground_type_grass;
            }
            break;
        }
        case g_ground_type_rock:
            break;
        case g_ground_type_grass://gets eaten if sheep close
        {
            for(size_t i = 0; i < entity_count; i++) {
                struct entity_t *e = entities + i;
                if(
                    e->type != g_entity_type_black_sheep &&
                    e->type != g_entity_type_white_sheep
                ) {
                    continue;
                }

                float d = hf_vec2_distance(tile_pos, e->pos);
                if(d <= .6f) {
                    tiles[index] = g_ground_type_dirt;
                }
            }            
            break;
        }
        default:
            break;
    }
}

static void load_map(const char* path) {
    FILE* file = fopen(path, "rb");
    if(!file) {
        return;
    }

    int tile_count = g_map_size * g_map_size;
    for(int i = 0; i < tile_count; i++) {//read tiles
        unsigned char val;
        fread(&val, sizeof(unsigned char), 1, file);
        tiles[i] = (g_ground_type)val;
    }

    g_entity_type read_entities[g_map_size * g_map_size];
    entity_count = 0u;
    for(int i = 0; i < tile_count; i++) {//read entities
        unsigned char val;
        fread(&val, sizeof(unsigned char), 1, file);
        read_entities[i] = (g_entity_type)val;
        if(read_entities[i]) {//does not consider entity_type_none
            entity_count++;
        }
    }
    fclose(file);

    entities = calloc(entity_count, sizeof(*entities));
    int entity_index = 0;
    for(int i = 0; i < tile_count; i++) {//passes through all entities and put valid ones in the array
        if(!read_entities[i]) {
            continue;
        }
        entities[entity_index].type = read_entities[i];
        entities[entity_index].pos = (hf_vec2) {
            .x = (float)(i % g_map_size),
            .y = (float)(i / g_map_size)
        };
        entities[entity_index].dir = (hf_vec2) { 0.f, 0.f };
        entity_index++;
    }
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

static int count_entities(g_entity_type type) {
    int count = 0;
    for(size_t i = 0; i < entity_count; i++) {
        if(entities[i].type == type) {
            count++;
        }
    }
    return count;
}

static int count_tiles(g_ground_type type) {
    int count = 0;
    for(size_t i = 0; i < g_map_size * g_map_size; i++) {
        if(tiles[i] == type) {
            count++;
        }
    }
    return count;
}

static bool is_entity_close_to_type(const struct entity_t* entity, g_entity_type type) {
    for(size_t i = 0; i < entity_count; i++) {
        struct entity_t* other = entities + i;
        if(other == entity) {
            continue;
        }

        if(other->type == type && hf_vec2_sqr_distance(entity->pos, other->pos) < (3.f * 3.f)) {
            return true;
        }
    }
    return false;
}

static bool is_sheep_close_to_flag(const struct entity_t *sheep) {
    if(sheep->type != g_entity_type_black_sheep && sheep->type != g_entity_type_white_sheep) {
        return false;
    }

    g_entity_type flag_type =
        sheep->type == g_entity_type_black_sheep ?
        g_entity_type_black_flag :
        g_entity_type_white_flag
    ;

    return is_entity_close_to_type(sheep, flag_type);
}

static int count_sheep_at_flag(g_entity_type sheep_type) {
    int count = 0;
    for(size_t i = 0; i < entity_count; i++) {
        struct entity_t *e = entities + i;
        if(entities[i].type == sheep_type && is_sheep_close_to_flag(e)) {
            count++;
        }
    }
    return count;
}

static void draw_entity(SDL_Renderer* renderer, const struct entity_t *entity) {
    SDL_Rect texture_rect = get_entity_texture_rect(entity->type);

    SDL_Rect dest_rect = {
        .x = (int)(entity->pos.x * (float)g_tile_size) - texture_rect.w / 2,//pivot center
        .y = (int)(entity->pos.y * (float)g_tile_size) - texture_rect.h,//pivot down
        .w = texture_rect.w,
        .h = texture_rect.h,
    };


    // TODO: draw shadow
    if(is_sheep_close_to_flag(entity)) {
        SDL_SetRenderDrawColor(renderer, 255, 255, 100, 255);
        SDL_RenderDrawRect(renderer, &dest_rect);
    }
    // TODO: animation??

    SDL_RenderCopy(renderer, entities_texture, &texture_rect, &dest_rect);
}

static void draw_map(SDL_Renderer* renderer) {
    if(is_night) {
        SDL_SetTextureColorMod(ground_texture, 160, 160, 220);
    }
    else {
        SDL_SetTextureColorMod(ground_texture, 255, 255, 255);
    }

    for(int x = 0; x < g_map_size; x++) {
        for(int y = 0; y < g_map_size; y++) {
            const int index = g_grid_to_index(x, y);
            const g_ground_type ground_type = tiles[index];
            SDL_Rect ground_src_rect = get_ground_texture_rect(ground_type);

            SDL_Rect tile_rect = g_get_tile_rect(x, y);
            SDL_RenderCopy(renderer, ground_texture, &ground_src_rect, &tile_rect);
        }
    }
}

static void draw_sheep_progress_bar(SDL_Renderer *renderer, SDL_Color color, g_entity_type type, int y) {
    int sheep_count = count_entities(type);
    float sheep_pct = 1.f;
    if(sheep_count) {
        sheep_pct = (float)count_sheep_at_flag(type) / (float)sheep_count;
    }

    int tex_w;
    hf_scene_get_render_size(&tex_w, NULL);
    int map_w = g_map_size * g_tile_size;
    SDL_Rect full_rect = {
        .x = map_w + progress_bar_padding,
        .y = y,
        tex_w - map_w - progress_bar_padding * 2,
        progress_bar_height
    };

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 100u);
    SDL_RenderFillRect(renderer, &full_rect);

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &full_rect);

    SDL_Rect pct_rect = {
        .x = full_rect.x,
        .y = full_rect.y,
        .w = (int)((float)full_rect.w * sheep_pct),
        .h = full_rect.h
    };
    SDL_RenderFillRect(renderer, &pct_rect);
}

static void draw_grass_progress_bar(SDL_Renderer *renderer, SDL_Color color, int y) {
    const int grass_count = count_tiles(g_ground_type_grass);
    const int dirt_count = count_tiles(g_ground_type_dirt);
    const int total_count = grass_count + dirt_count;
    const float pct = (float)dirt_count / (float)total_count;

    int tex_w;
    hf_scene_get_render_size(&tex_w, NULL);
    int map_w = g_map_size * g_tile_size;
    SDL_Rect full_rect = {
        .x = map_w + progress_bar_padding,
        .y = y,
        tex_w - map_w - progress_bar_padding * 2,
        progress_bar_height
    };

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, 100u);
    SDL_RenderFillRect(renderer, &full_rect);

    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(renderer, &full_rect);

    SDL_Rect pct_rect = {
        .x = full_rect.x,
        .y = full_rect.y,
        .w = (int)((float)full_rect.w * pct),
        .h = full_rect.h
    };
    SDL_RenderFillRect(renderer, &pct_rect);
}

static void toggle_day_night() {
    is_night = !is_night;
}

static void reload_scene() {
    hf_scene_load(g_scene_get_game());
}

static void goto_menu() {
    hf_scene_load(g_scene_get_menu());
}

//generic scene functions
static void init(void) {
    load_map("./maps/editor.map");

    SDL_Renderer* renderer = hf_scene_get_renderer();
    entities_texture = IMG_LoadTexture(renderer, "./res/entities.png");
    ground_texture = IMG_LoadTexture(renderer, "./res/ground.png");
    buttons_texture = IMG_LoadTexture(renderer, "./res/buttons.png");

    //setup buttons
    int button_h;
    SDL_QueryTexture(buttons_texture, NULL, NULL, NULL, &button_h);
    for(int i = 0; i < button_count; i++) {
        buttons[i].src_rect = (SDL_Rect) {
            .x = button_h * i,
            .y = 0,
            .w = button_h,
            .h = button_h
        };
        buttons[i].on_click = NULL;
    }

    int tex_w;
    hf_scene_get_render_size(&tex_w, NULL);
    int map_w = g_tile_size * g_map_size;
    int diff_w = tex_w - map_w;

    buttons[button_id_daycycle].on_click = toggle_day_night;
    buttons[button_id_daycycle].dest_rect = (SDL_Rect) {
        .x = map_w + diff_w / 2 - button_h / 2,
        .y = 120,
        .w = button_h,
        .h = button_h
    };
    buttons[button_id_restart].on_click = reload_scene;
    buttons[button_id_restart].dest_rect = (SDL_Rect) {
        .x = map_w + diff_w / 4 - button_h / 2,
        .y = 400,
        .w = button_h,
        .h = button_h
    };
    buttons[button_id_home].on_click = goto_menu;
    buttons[button_id_home].dest_rect = (SDL_Rect) {
        .x = map_w + (diff_w / 4) * 3 - button_h / 2,
        .y = 400,
        .w = button_h,
        .h = button_h
    };

    is_night = false;
}

static void quit(void) {
    free(entities);

    SDL_DestroyTexture(ground_texture);
    SDL_DestroyTexture(entities_texture);
    SDL_DestroyTexture(buttons_texture);
}

static void process_input(SDL_Event e) {
    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    int tex_x, tex_y;
    hf_scene_window_pos_to_render_pos(mouse_x, mouse_y, &tex_x, &tex_y);
    SDL_Point p = { .x = tex_x, .y = tex_y };

    switch(e.type) {
        case SDL_KEYDOWN:

            break;
        case SDL_MOUSEBUTTONDOWN:
            for(int i = 0; i < button_count; i++) {
                if(SDL_PointInRect(&p, &buttons[i].dest_rect)) {//clicked button
                    if(buttons[i].on_click) {
                        buttons[i].on_click();
                    }
                }    
            }
            break;
    }
}

static void update(float delta_time) {
    update_entities(delta_time);

    const int num_ticks = 10;
    for(int i = 0; i < num_ticks; i++) {
        int x = rand() % g_map_size;
        int y = rand() % g_map_size;

        tick_tile(x, y);
    }
}

static void render(SDL_Renderer* renderer) {
    SDL_SetRenderDrawColor(renderer, 100, 100, 255, 255);
    SDL_RenderClear(renderer);    

    draw_map(renderer);

    // TODO: bubble sort from to to bot
    for(size_t i = 0u; i < entity_count; i++) {
        draw_entity(renderer, entities + i);
    }

    //draw progression bars
    draw_sheep_progress_bar(renderer, (SDL_Color){ 255, 255, 255, 255 }, g_entity_type_white_sheep, 10);
    draw_sheep_progress_bar(renderer, (SDL_Color){   0,   0,   0, 255 }, g_entity_type_black_sheep, 40);
    draw_grass_progress_bar(renderer, (SDL_Color){  50, 255,  50, 255}, 70);

    int mouse_x, mouse_y;
    SDL_GetMouseState(&mouse_x, &mouse_y);
    int tex_x, tex_y;
    hf_scene_window_pos_to_render_pos(mouse_x, mouse_y, &tex_x, &tex_y);
    SDL_Point p = { .x = tex_x, .y = tex_y };
    for(int i = 0; i < button_count; i++) {
        if(SDL_PointInRect(&p, &buttons[i].dest_rect)) {
            SDL_SetTextureColorMod(buttons_texture, 255, 255, 255);
        }
        else {
            SDL_SetTextureColorMod(buttons_texture, 150, 150, 150);
        }

        SDL_RenderCopy(renderer, buttons_texture, &buttons[i].src_rect, &buttons[i].dest_rect);
    }
}

hf_scene  g_scene_get_game(void) {
    hf_scene new_scene = {
        .init = init,
        .quit = quit,
        .process_input = process_input,
        .update = update,
        .render = render
    };
    return new_scene;
}