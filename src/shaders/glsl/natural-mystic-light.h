// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_LIGHT_H_INCLUDED)
#define NATURAL_MYSTIC_LIGHT_H_INCLUDED 1

#include "natural-mystic-color.h"
#include "natural-mystic-config.h"
#include "natural-mystic-noise.h"

/* Light color constants. Should be private to this file.
 */
const vec3 torchlightColor = vec3(1.0, 0.66, 0.28);
const vec3 skylightColor   = vec3(0.8392, 0.9098, 0.9961);
const vec3 moonlightColor  = vec3(112.0, 135.5, 255.0)/255.0;

/* Calculate the color of sunlight based on the time-dependent
 * daylight level "daylight" [0, 1]. The color of sunlight changes
 * depending on the daylight level to express dusk and dawn.
 */
vec3 sunlightColor(float daylight) {
    const vec3 setColor = vec3(1.0, 0.3569, 0.0196);
    const vec3 dayColor = vec3(1.0, 0.8706, 0.8039);

    return mix(setColor, dayColor, smoothstep(0.45, 1.0, daylight));
}

/* Calculate the color of the ambient light based solely on the fog
 * color.
 */
vec3 ambientLightColor(vec4 fogColor) {
    return brighten(fogColor.rgb);
}

/* Calculate the color of the ambient light based on the
 * terrain-dependent sunlight level, and the time-dependent daylight
 * level. The level of ambient light is not dependent on the terrain
 * but the color is.
 */
vec3 ambientLightColor(float sunLevel, float daylight) {
    /* The daylight color is a mixture of sunlight and skylight. */
    vec3 daylightColor = mix(skylightColor, sunlightColor(daylight), 0.625);

    /* The influence of the sun and the moon depends on the daylight
     * level. */
    vec3 outsideColor = mix(moonlightColor, daylightColor, daylight);

    /* In caves the torch light is the only possible light source but
     * on the ground the sun or the moon is the most influential. */
    return brighten(mix(torchlightColor, outsideColor, sunLevel));
}

/* Apply the ambient light on the original fragment "frag". The .a
 * component denotes the intensity. Without this filter, objects
 * getting no light will be rendered in complete darkness, which isn't
 * how the reality works.
 */
vec3 applyAmbientLight(vec3 frag, vec3 pigment, vec3 lightColor, float intensity) {
    return frag + intensity * pigment * lightColor;
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
        return frag + pigment * torchlightColor * intensity;
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
    const float baseIntensity = 50.0;
    const float shadowFactor  = 0.01;  // [0, 1]
    const float shadowBorder  = 0.87;  // [0, 1]
    const float shadowBlur    = 0.003; // The higher the more blur.

    float intensity = baseIntensity * sunLevel * daylight;
    if (intensity > 0.0) {
        /* Shadows reduce the amount of sunlight. The reason why we
         * don't remove it entirely is that a shadowed area near a lit
         * area will receive higher amount of scattered light. If it
         * were completely occluded the sunlight level won't be
         * non-zero. */
        intensity *= mix(
            shadowFactor, 1.0,
            smoothstep(shadowBorder - shadowBlur, shadowBorder + shadowBlur, sunLevel));

        return frag + pigment * sunlightColor(daylight) * intensity;
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
    const float baseIntensity = 30.0;

    float intensity = baseIntensity * sunLevel * daylight;
    if (intensity > 0.0) {
        return frag + pigment * skylightColor * intensity;
    }
    else {
        return frag;
    }
}

/* Apply the moonlight on a fragment "frag" based on the
 * time-dependent daylight level [0, 1] and the terrain-dependent
 * sunlight level [0, 1]. The moonlight behaves like sunlight but is
 * blue-ish white.
 */
vec3 applyMoonlight(vec3 frag, vec3 pigment, float sunLevel, float daylight) {
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

        return frag + pigment * moonlightColor * intensity;
    }
    else {
        return frag;
    }
}

#endif
