#include <hf/hf_vec2.h>

hf_vec2 hf_vec2_add(hf_vec2 a, hf_vec2 b) {
    hf_vec2 res = {
        .x = a.x + b.x,
        .y = a.y + b.y
    };
    return res;
}

hf_vec2 hf_vec2_sub(hf_vec2 a, hf_vec2 b) {
    hf_vec2 res = {
        .x = a.x - b.x,
        .y = a.y - b.y
    };
    return res;
}

hf_vec2 hf_vec2_mul(hf_vec2 vec, float v) {
    hf_vec2 res = {
        .x = vec.x * v,
        .y = vec.y * v
    };
    return res;
}

hf_vec2 hf_vec2_div(hf_vec2 vec, float v) {
    hf_vec2 res = {
        .x = vec.x / v,
        .y = vec.y / v
    };
    return res;
}