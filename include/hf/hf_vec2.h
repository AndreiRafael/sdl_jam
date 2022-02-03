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

#endif//HF_VEC2_H