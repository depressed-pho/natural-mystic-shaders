// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_LIGHT_H_INCLUDED)
#define NATURAL_MYSTIC_LIGHT_H_INCLUDED 1

#include "natural-mystic-color.h"
#include "natural-mystic-config.h"
#include "natural-mystic-noise.h"

/* Calculate the color and the intensity of the ambient light, based
 * on the fog color. */
vec4 ambientLight(vec4 fogColor) {
    const vec3  baseColor = vec3(1.0);
    const float intensity = 6.0;

    vec3 color = mix(baseColor, brighten(fogColor.rgb), fogColor.a);
    return vec4(color, intensity);
}

/* Apply the ambient light on the original fragment "frag". The .a
 * component denotes the intensity. Without this filter, objects
 * getting no light will be rendered in complete darkness, which isn't
 * how the reality works.
 */
vec3 applyAmbientLight(vec3 frag, vec3 pigment, vec4 ambient) {
    return frag + ambient.a * pigment * ambient.rgb;
}

/* Calculate the torch light flickering factor [-1, 1] based on the
 * in-game time.
 */
float torchLightFlicker(highp float time) {
    /* The flicker factor is solely determined by the time. Ideally it
     * should be separately computed for each light source in the
     * scene, but we can't do it because shaders don't have access to
     * such information.
     */
    highp float amplitude = 0.15;
    highp float st        = time * 3.0;
    highp float flicker   = clamp(perlinNoise(st), 0.0, 1.0);
    return (flicker * 2.0 - 1.0) * amplitude;
}

/* Calculate and apply the torch light on the original fragment
 * "frag". The argument "torchLevel" should be the torch light level
 * [0, 1], and the "time" is the in-game time, used for the flickering
 * effect.
 */
vec3 applyTorchLight(vec3 frag, vec3 pigment, float torchLevel, float sunLevel, float daylight, highp float time) {
    const vec3  torchColor    = vec3(1.0, 0.66, 0.28);
    const float baseIntensity = 160.0;
    const float decay         = 5.0;

    if (torchLevel > 0.0) {
        float intensity = baseIntensity * pow(torchLevel, decay);

        /* Reduce the effect of the torch light on areas lit by the
         * sunlight. Theoretically we shouldn't need to do this and
         * instead can use much more intense light for the sun, but we
         * haven't found a good tone mapping curve for that.
         */
        intensity *= mix(1.0, 0.1, smoothstep(0.65, 0.875, sunLevel * daylight));

#if defined(ENABLE_TORCH_FLICKER)
        intensity *= torchLightFlicker(time) + 1.3;
#endif
        return frag + pigment * torchColor * intensity;
    }
    else {
        return frag;
    }
}

/* Apply the sunlight on a fragment "frag" based on the
 * terrain-dependent sunlight level [0,1] and the time-dependent
 * daylight level "daylight" [0,1]. The sunlight is yellow-ish
 * red. The sunlight comes from the sun which behaves like a
 * directional light.
 */
vec3 applySunlight(vec3 frag, vec3 pigment, float sunLevel, float daylight) {
    const vec3  setColor      = vec3(1.0, 0.3569, 0.0196);
    const vec3  dayColor      = vec3(1.0, 0.8706, 0.8039);
    const float baseIntensity = 50.0;
    const float shadowFactor  = 0.10;  // [0, 1]
    const float shadowBorder  = 0.87;  // [0, 1]
    const float shadowBlur    = 0.003; // The higher the more blur.

    float intensity = baseIntensity * sunLevel * daylight;
    if (intensity > 0.0) {
        /* The color of sunlight changes depending on the daylight
         * level to express dusk and dawn. */
        vec3 sunColor = mix(setColor, dayColor, smoothstep(0.4, 1.0, daylight));

        /* Shadows reduce the amount of sunlight. */
        intensity *= mix(
            shadowFactor, 1.0,
            smoothstep(shadowBorder - shadowBlur, shadowBorder + shadowBlur, sunLevel));

        return frag + pigment * sunColor * intensity;
    }
    else {
        return frag;
    }
}

/* Apply the skylight on a fragment "frag" based on the
 * terrain-dependent sunlight level [0,1] and the time-dependent
 * daylight level "daylight" [0,1]. The skylight is blue-ish
 * white. The skylight comes from the sky which behaves like an
 * ambient light but is affected by occlusion.
 */
vec3 applySkylight(vec3 frag, vec3 pigment, float sunLevel, float daylight) {
    const vec3 skyColor = vec3(0.8392, 0.9098, 0.9961);
    const float baseIntensity = 30.0;

    float intensity = baseIntensity * sunLevel * daylight;
    if (intensity > 0.0) {
        return frag + pigment * skyColor * intensity;
    }
    else {
        return frag;
    }
}

/* Apply the moonlight on a fragment "frag" based on the
 * time-dependent daylight level [0,1] and the terrain-dependent
 * sunlight level [0,1]. The moonlight behaves like sunlight but is
 * blue-ish white.
 */
vec3 applyMoonlight(vec3 frag, vec3 pigment, float sunLevel, float daylight) {
    const vec3 moonColor = vec3(0.1, 0.2431, 1.0);
    const float baseIntensity = 10.0;
    const float shadowFactor  = 0.20;  // [0, 1]
    const float shadowBorder  = 0.87;  // [0, 1]
    const float shadowBlur    = 0.003; // The higher the more blur.

    float intensity = baseIntensity * sunLevel * (1.0 - daylight);
    if (intensity > 0.0) {
        /* Shadows reduce the amount of moonlight. */
        intensity *= mix(
            shadowFactor, 1.0,
            smoothstep(shadowBorder - shadowBlur, shadowBorder + shadowBlur, sunLevel));

        return frag + pigment * moonColor * intensity;
    }
    else {
        return frag;
    }
}

#endif
