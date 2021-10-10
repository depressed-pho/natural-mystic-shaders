// -*- hlsl -*-
#if !defined(NATURAL_MYSTIC_HACKS_FXH_INCLUDED)
#define NATURAL_MYSTIC_HACKS_FXH_INCLUDED 1

#include "natural-mystic-color.fxh"

/* Detect grasses based on the color of material. The color must be in
 * HSV, not RGB. This function also returns true for grass blocks and
 * leaves because it's impossible to tell them from plants.
 */
bool isGrass(float3 hsv) {
    /* Grass colors are affected by seasons and can turn red. */
    float hue = hsv.x * 360.0;
    return hsv.y > 0.1 && (hue < 149.0 && hue > 12.0);
}

/* Detect water based on the color of material. The color must be in
 * HSV, not RGB.
 */
bool isWater(float3 hsv) {
    /* Yikes. The color of water greatly changes in swampland so we
     * need an HSV value of the color to reliably detect water.
     */
    float hue = hsv.x * 360.0;
    return hsv.y > 0.1 && hue >= 147.0 && hue <= 270.0;
}

/* Detect a water plane based on the world position of a vertex. */
bool isWaterPlane(float4 wPos) {
    float y = frac(wPos.y);
    return y >= 0.7 && y <= 0.9;
}

/* Detect the Nether fog. [Currently unused] */
bool isNetherFog(float4 fogColor) {
    return fogColor.r > fogColor.b && fogColor.r < 0.5 && fogColor.b < 0.05;
}

/* Detect the End fog. [Currently unused] */
bool isTheEndFog(float4 fogColor) {
    return fogColor.r > fogColor.g && fogColor.b > fogColor.g &&
        (fogColor.r < 0.05 == true && fogColor.g < 0.05 == true && fogColor.b < 0.05 == true);
}

/* Detect the render distance fog. NOTE: It is tempting to use
 * material variants "*_far" instead of this hack (see
 * terrain.material) but no, that's actually not possible because the
 * fog they can have is not always the render distance fog. It can
 * also be the bad weather fog.
 */
bool isRenderDistanceFog(float2 fogControl) {
    return fogControl.x > 0.6;
}

/* When it's raining on the Overworld, the game gradually reduces the
 * fog far to < 1.0. We exploit this fact to detect rain. This
 * function returns 0.0 when it's raining, and 1.0 otherwise.
 */
float isClearWeather(float2 fogControl) {
    return smoothstep(0.8, 1.0, fogControl.y);
}

/* When it's near dusk or dawn, the B component of the fog color
 * gradually decreases. We exploit this fact to detect dusk and
 * dawn. Terrain shaders don't need this function because they can
 * directly fetch the daylight level. This function returns 1.0 when
 * it's dusk or dawn, or 0.0 otherwise.
 */
float isDuskOrDawn(float4 fogColor) {
    return pow(saturate(1.0 - fogColor.b * 1.7), 0.3);
}

/* When it's night, the R component of the fog color significantly
 * decreases. We exploit this fact to detect night. Terrain shaders
 * don't need this function because they can directry fetch the
 * daylight level. This function returns 1.0 when it's night, or 0.0
 * otherwise.
*/
float isNight(float4 fogColor) {
    return pow(saturate(1.0 - fogColor.r * 1.5), 2.0);
}

/* Compute an occlusion factor [0, 1] based on the vertex color. 0.0
 * means completely occluded, and 1.0 means not occluded at all. This
 * works because the game appears to encode an ambient occlusion in
 * the vertex color unless SEASONS is defined.
 */
float occlusionFactor(float3 color) {
    static const float shadowBorder = 0.83;
    static const float shadowBlurLo = 0.05;
    static const float shadowBlurHi = 0.01;

    /* Hackish adjustment for grass blocks! */
    float luminance = color.g * 2.0 - (color.r < color.b ? color.r : color.b);

    return smoothstep(shadowBorder - shadowBlurLo, shadowBorder + shadowBlurHi, luminance);
}

#endif /* NATURAL_MYSTIC_HACKS_FXH_INCLUDED */
