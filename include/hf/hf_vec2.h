#ifndef HF_VEC2_H
#define HF_VEC2_H

typedef struct hf_vec2_t {
    float x;
    float y;
} hf_vec2;

hf_vec2 hf_vec2_add(hf_vec2 a, hf_vec2 b);
hf_vec2 hf_vec2_sub(hf_vec2 a, hf_vec2 b);
hf_vec2 hf_vec2_mul(hf_vec2 vec, float v);
hf_vec2 hf_vec2_div(hf_vec2 vec, float v);
float hf_vec2_sqr_magnitude(hf_vec2 v);
float hf_vec2_magnitude(hf_vec2 v);
float hf_vec2_sqr_distance(hf_vec2 a, hf_vec2 b);
float hf_vec2_distance(hf_vec2 a, hf_vec2 b);
hf_vec2 hf_vec2_lerp(hf_vec2 a, hf_vec2 b, float t);
hf_vec2 hf_vec2_normalized(hf_vec2 v);

#endif//HF_VEC2_H