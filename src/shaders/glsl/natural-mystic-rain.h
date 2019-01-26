// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_RAIN_H_INCLUDED)
#define NATURAL_MYSTIC_RAIN_H_INCLUDED 1

#include "natural-mystic-noise.h"

/* See https://seblagarde.wordpress.com/2012/12/10/observe-rainy-world/
 */

/* Compute the wetness of the terrain based on the clear weather level
 * [0,1] and terrain-dependent sunlight level [0,1].
 */
float wetness(float clearWeather, float sunLevel) {
    const float shadowFactor = 0.01;  // [0, 1]
    const float shadowBorder = 0.80;  // [0, 1]
    const float shadowBlur   = 0.06; // The higher the more blur.

    return (1.0 - clearWeather) * smoothstep(shadowBorder - shadowBlur, shadowBorder + shadowBlur, sunLevel);
}

vec3 ripples(vec3 incomingLight, highp vec3 worldPos, highp float time, highp vec3 normal) {
    /* Water ripples should only be apparent on the ground, i.e. where the
     * normal matches to (0, 1, 0). */
    float cosTheta = max(0.0, normal.y); // Equivalent to max(0.0, dot(normal, vec3(0, 1, 0)));

    const highp vec3 resolution = vec3(vec2(0.15), 0.5);
    const float amount = 0.1;

    highp vec3 st = vec3(worldPos.xz, time) / resolution;
    float ripples = (simplexNoise(st) + 0.8) * 0.5;

    return mix(0.2, 1.0, cosTheta) * incomingLight * ripples * amount;
}

#endif /* defined(NATURAL_MYSTIC_RAIN_H_INCLUDED) */
