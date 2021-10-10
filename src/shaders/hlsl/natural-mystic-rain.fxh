// -*- hlsl -*-
#if !defined(NATURAL_MYSTIC_RAIN_FXH_INCLUDED)
#define NATURAL_MYSTIC_RAIN_FXH_INCLUDED 1

#include "natural-mystic-noise.fxh"

/* See https://seblagarde.wordpress.com/2012/12/10/observe-rainy-world/
 */

/* Compute the wetness of the terrain based on the clear weather level
 * [0,1] and terrain-dependent sunlight level [0,1].
 */
float wetness(float clearWeather, float sunLevel) {
    static const float shadowFactor = 0.01;  // [0, 1]
    static const float shadowBorder = 0.80;  // [0, 1]
    static const float shadowBlur   = 0.06; // The higher the more blur.

    return (1.0 - clearWeather) * smoothstep(shadowBorder - shadowBlur, shadowBorder + shadowBlur, sunLevel);
}

/* Compute light reflected by water ripples on the ground.
 */
float3 ripples(float3 incomingLight, float3 worldPos, float cameraDist, float time, float3 normal) {
    /* The visual effect of ripples is so subtle, and it won't be
     * visible on far terrain. We can skip the costly noise generation
     * unless worldPos isn't close to the camera. */
    static const float distThreshold = 0.1;
    static const float distFadeStart = distThreshold * 0.8;

    if (cameraDist < distThreshold) {
        /* Water ripples should only be apparent on the ground, i.e. where the
         * normal matches to (0, 1, 0). */
        float cosTheta = max(0.0, normal.y); // Equivalent to max(0.0, dot(normal, float3(0, 1, 0)));

        static const float3 resolution = float3(0.16, 0.16, 0.5);
        static const float amount = 0.1;

        float3 st = float3(worldPos.xz, time) / resolution;
        float ripples = simplexNoise(st);

        /* Shift the range of ripples. */
        ripples = (ripples + 0.8) * 0.5;

        /* Threshold and scale of ripples. */
        ripples = smoothstep(0.3, 1.0, ripples);

        return incomingLight * lerp(0.2, 1.0, cosTheta) * ripples * amount *
            (1.0 - smoothstep(distFadeStart, distThreshold, cameraDist));
    }
    else {
        return 0.0;
    }
}

#endif /* defined(NATURAL_MYSTIC_RAIN_FXH_INCLUDED) */
