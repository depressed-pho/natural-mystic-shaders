// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_HACKS_H_INCLUDED)
#define NATURAL_MYSTIC_HACKS_H_INCLUDED 1

/* Detect leaves based on the color of material. */
bool isLeaf(vec4 color) {
    vec4 norm = normalize(color);
    return norm.g >= 0.65 && norm.g <= 0.9;
}

/* Detect water based on the color of material. */
bool isWater(vec4 color) {
    vec4 norm = normalize(color);
    return norm.b >= 0.65 && norm.b <= 0.9;
}

#endif /* NATURAL_MYSTIC_HACKS_H_INCLUDED */
