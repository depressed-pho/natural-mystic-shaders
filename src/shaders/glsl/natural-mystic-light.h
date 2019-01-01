// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_LIGHT_H_INCLUDED)
#define NATURAL_MYSTIC_LIGHT_H_INCLUDED 1

#include "natural-mystic-color.h"
#include "natural-mystic-config.h"
#include "natural-mystic-noise.h"

/* Calculate the color and the intensity of the ambient light, based
 * on the fog color. */
vec4 ambientLight(vec4 fogColor) {
    /* THINKME: The existence of fog should increase the intensity of
     * ambient light (#32). */
    return vec4(
        mix(vec3(1.0), brighten(fogColor.rgb), fogColor.a),
        0.1);
}

/* Apply the ambient light on the original fragment "frag". The .a
 * component denotes the intensity. Without this filter, objects
 * getting no light will be rendered in complete darkness, which isn't
 * how the reality works.
 */
vec3 applyAmbientLight(vec3 frag, vec3 pigment, vec4 ambient) {
    return frag + ambient.a * pigment * ambient.rgb;
}

/* Calculate and apply a shadow on the original fragment "frag". The
 * argument "torchLevel" should be the torch light level [0, 1],
 * "sunLevel" should be the terrain-dependent sunlight level [0, 1],
 * and "daylight" should be the time-dependent daylight level.
 */
vec3 applyShadow(vec3 frag, float torchLevel, float sunLevel, float daylight, float baseDensity) {
    const vec3 shadowColor = vec3(0.0);
    const float minShadow = 0.0;  // [0, 1]
    const float maxShadow = 0.45; // [0, 1]
    const float blur = 0.003; // The higher the more blur.
    const float torchLightCutOff = 0.1; // [0, 1]

    /* The less the sunlight level is, the darker the fragment will
     * be. */
    float density = mix(maxShadow, minShadow,
                        smoothstep(
                            0.870 - blur,
                            0.870 + blur,
                            sunLevel));

    if (density > 0.0) {
        /* The existence of torch light should negate the effect of
         * shadows. The higher the torch light level is, the less the
         * shadow affects the color. However, we also don't want
         * torches to completely drive away the shadow, since the
         * intensity of the torch light wouldn't be comparable to that
         * of sunlight.
         */
        density *= mix(1.0, torchLightCutOff,
                       smoothstep(0.5, 1.0, torchLevel));

        /* NOTE: Lack of daylight should also negate the effect of
         * shadows, because sadly the light map passed by the upstream
         * doesn't take torches into account. But we can't, because if
         * we reduce "density" depending on "daylight", caves will get
         * brighter at night. We could possibly overcome the issue by
         * completely disabling the vanilla lighting (i.e. diffuse *=
         * texture2D(TEXTURE_1, uv1)) and replacing it with an HDR
         * lighting.
         */
        return mix(frag, shadowColor, baseDensity * density);
    }
    else {
        return frag;
    }
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
    highp float amplitude = 0.2;
    highp float st        = time * 3.0;
    highp float flicker   = clamp(perlinNoise(st), 0.0, 1.0);
    return (flicker * 2.0 - 1.0) * amplitude;
}

/* Calculate and apply the torch color on the original fragment
 * "frag". The argument "torchLevel" should be the torch light level
 * [0, 1], "sunLevel" should be the terrain-dependent sunlight level
 * [0, 1], and "daylight" should be the time-dependent daylight
 * level. The "time" is the in-game time, used for the flickering
 * effect.
 */
vec3 applyTorchColor(vec3 frag, float torchLevel, float sunLevel, float daylight, highp float time) {
    const vec3 torchColor = vec3(0.8, 0.3, -0.2);
    const float torchDecay = 0.55; // [0, 1]
    const float baseIntensity = 1.0; // [0, 1]
    const float sunlightCutOff = 0.1; // [0, 1]

    /* The sunlight should prevent torches from affecting the color,
     * but we also have to take the daylight level into account.
     */
    float intensity = max(0.0, torchLevel - torchDecay) * baseIntensity;
    if (intensity > 0.0) {
        intensity *= mix(1.0, sunlightCutOff, smoothstep(0.65, 0.875, sunLevel * daylight));
#if defined(ENABLE_TORCH_FLICKER)
        intensity *= torchLightFlicker(time) + 1.3;
#endif
        return frag + torchColor * intensity;
    }
    else {
        return frag;
    }
}

/* Apply the sunlight on a fragment "frag" based on the time-dependent
 * daylight level "daylight" [0,1]. The argument "sunLevel" should be
 * the terrain-dependent sunlight level [0,1]. The sunlight is
 * yellow-ish red.
 */
vec3 applySunlight(vec3 frag, float sunLevel, float daylight) {
    const vec3 sunColor = vec3(0.7, 0.3, 0.0);

    float intensity = (0.5 - abs(0.5 - daylight)) * sunLevel;
    if (intensity > 0.0) {
        float sunset = dot(frag, vec3(1.0)) / 1.5;
        return mix(frag, sunset * sunColor, intensity);
    }
    else {
        return frag;
    }
}

/* Apply the skylight on a fragment "frag" based on the time-dependent
 * daylight level "daylight" [0,1]. The argument "sunLevel" should be
 * the terrain-dependent sunlight level [0,1]. The skylight is
 * blue-ish white.
 */
vec3 applySkylight(vec3 frag, float sunLevel, float daylight) {
    const vec3 skyColor = vec3(0.8, 0.8, 1.0);
    const float skyLevel = 0.07;

    float intensity = sunLevel * skyLevel * daylight;

    return frag + skyColor * intensity;
}

/* Apply the moonlight on a fragment "frag" based on the
 * time-dependent daylight level "daylight" [0,1]. The argument
 * "torchLevel" should be the torch light level [0,1], and the
 * argument "sunLevel" should be the terrain-dependent sunlight level
 * [0,1]. The moonlight is purple-ish white.
 */
vec3 applyMoonlight(vec3 frag, float torchLevel, float sunLevel, float daylight) {
    const vec3 moonColor = vec3(0.5, 0.8, 0.95);
    const float moonLevel = 0.7;

    /* The reason why we take into account the torch light level is
     * that the intensity of the torch light is far higher than that
     * of moonlight.
     */
    float factor = sunLevel * (1.0 - torchLevel) * moonLevel * (1.0 - daylight);
    if (factor > 0.0) {
        return mix(frag, desaturate(frag, 0.8) * moonColor, factor);
    }
    else {
        return frag;
    }
}

#endif
