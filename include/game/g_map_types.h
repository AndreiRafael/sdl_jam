#ifndef G_MAP_TYPES_H
#define G_MAP_TYPES_H

typedef enum g_ground_type_t {
    g_ground_type_grass,
    g_ground_type_dirt,
    g_ground_type_rock,
    g_ground_type_max_value,
} g_ground_type;

typedef enum g_entity_type_t {
    g_entity_type_none = 0,

    g_entity_type_white_sheep,
    g_entity_type_white_shepherd,
    g_entity_type_white_flag,

    g_entity_type_black_sheep,
    g_entity_type_black_shepherd,
    g_entity_type_black_flag,

    g_entity_type_wolf,

    g_entity_type_max_value,
} g_entity_type;

enum {
    g_map_size = 20,
    g_tile_size = 40,
};

#endif//G_MAP_TYPES_H