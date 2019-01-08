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

/* Detect the render distance fog. THINKME: We could possibly use the
 * material variant "fading" instead of this hack. See
 * terrain.material
 */
bool isRenderDistanceFog(vec2 fogControl) {
    return fogControl.x > 0.5;
}

#endif /* NATURAL_MYSTIC_HACKS_H_INCLUDED */
