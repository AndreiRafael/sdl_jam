#include <hf/hf_vec2.h>
#include <math.h>

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

float hf_vec2_sqr_magnitude(hf_vec2 v) {
    return v.x * v.x + v.y * v.y;
}

float hf_vec2_magnitude(hf_vec2 v) {
    return sqrtf(hf_vec2_sqr_magnitude(v));
}

float hf_vec2_sqr_distance(hf_vec2 a, hf_vec2 b) {
    const hf_vec2 diff = { .x = a.x - b.x, .y = a.y - b.y };
    return diff.x * diff.x + diff.y * diff.y;
}

float hf_vec2_distance(hf_vec2 a, hf_vec2 b) {
    return sqrtf(hf_vec2_sqr_distance(a, b));
}

hf_vec2 hf_vec2_lerp(hf_vec2 a, hf_vec2 b, float t) {
    return hf_vec2_add(hf_vec2_mul(a, 1.f - t), hf_vec2_mul(b, t));
}

hf_vec2 hf_vec2_normalized(hf_vec2 v) {
    const float mag = hf_vec2_magnitude(v);
    if(mag > 0.f) {
        return hf_vec2_div(v, mag);
    }
    return (hf_vec2) { .x = 0.f, .y = 0.f };
}