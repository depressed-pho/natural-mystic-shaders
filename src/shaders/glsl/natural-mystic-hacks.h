// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_HACKS_H_INCLUDED)
#define NATURAL_MYSTIC_HACKS_H_INCLUDED 1

#include "natural-mystic-color.h"

/* Detect leaves based on the color of material. */
bool isLeaf(vec4 color) {
    /* FIXME: This function currently fails to detect leaves whose
     * color is affected by seasons. */
    vec4 norm = normalize(color);
    return norm.g >= 0.65 && norm.g <= 0.9;
}

/* Detect grasses based on the color of material. This function also
 * returns true for grass blocks and leaves because it's impossible to
 * tell them from plants. [Currently unused]
 */
bool isGrass(vec4 color) {
    /* Grass colors are affected by seasons and can turn red. */
    vec4 hsv = rgb2hsv(color);
    return hsv.x >= 149.0 && hsv.x <= 270.0;
}

/* Detect water based on the color of material. */
bool isWater(vec4 color) {
    /* Yikes. The color of water greatly changes in swampland so we
     * need an HSV value of the color to reliably detect water.
     */
    vec4 hsv = rgb2hsv(color);
    return hsv.x >= 149.0 && hsv.x <= 270.0;
}

/* Detect a water plane based on the world position of a vertex. */
bool isWaterPlane(highp vec4 wPos) {
    highp float y = fract(wPos.y);
    return y >= 0.7 && y <= 0.9;
}

/* Detect the Nether fog. [Currently unused] */
bool isNetherFog(vec4 fogColor) {
    return fogColor.r > fogColor.b && fogColor.r < 0.5 && fogColor.b < 0.05;
}

/* Detect the End fog. [Currently unused] */
bool isTheEndFog(vec4 fogColor) {
    return fogColor.r > fogColor.g && fogColor.b > fogColor.g &&
        lessThan(fogColor.rgb, vec3(0.05)) == bvec3(true);
}

/* Detect the render distance fog. NOTE: It is tempting to use
 * material variants "*_far" instead of this hack (see
 * terrain.material) but no, that's actually not possible because the
 * fog they can have is not always the render distance fog. It can
 * also be the bad weather fog.
 */
bool isRenderDistanceFog(vec2 fogControl) {
    return fogControl.x > 0.6;
}

/* When it's raining on the Overworld, the game gradually reduces the
 * fog far to < 1.0. We exploit this fact to detect rain. This
 * function returns 0.0 when it's raining, and 1.0 otherwise.
 */
float isClearWeather(vec2 fogControl) {
    return smoothstep(0.8, 1.0, fogControl.y);
}

/* Compute an occlusion factor [0, 1] based on the vertex color. 0.0
 * means completely occluded, and 1.0 means not occluded at all. This
 * works because the game appears to encode an ambient occlusion in
 * the vertex color unless SEASONS is defined.
 */
float occlusionFactor(vec3 color) {
    const float shadowBorder = 0.83;
    const float shadowBlurLo = 0.05;
    const float shadowBlurHi = 0.01;

    /* Hackish adjustment for grass blocks! */
    float luminance = color.g * 2.0 - (color.r < color.b ? color.r : color.b);

    return smoothstep(shadowBorder - shadowBlurLo, shadowBorder + shadowBlurHi, luminance);
}

#endif /* NATURAL_MYSTIC_HACKS_H_INCLUDED */
