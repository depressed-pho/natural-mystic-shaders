// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_LIGHT_H_INCLUDED)
#define NATURAL_MYSTIC_LIGHT_H_INCLUDED 1

#include "natural-mystic-color.h"
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

/* Compute the ambient light to be accumulated to a fragment. Without
 * this filter, objects getting no light will be rendered in complete
 * darkness, which isn't how the reality works.
 */
vec3 ambientLight(vec3 lightColor, float intensity) {
    return lightColor * intensity;
}

/* Calculate the torch light flickering factor [0, 2] based on the
 * world coordinates and the in-game time.
 */
float torchLightFlicker(highp vec3 wPos, highp float time) {
    /* The flicker factor is solely determined by the coords and the
     * time. Ideally it should be separately computed for each light
     * source in the scene, but we can't do it because shaders don't
     * have access to such information. We instead generate a
     * 4-dimensional simplex noise and slice it by time.
     */
    const highp float amplitude  = 0.40;
    const highp vec4  resolution = vec4(vec3(12.0), 0.8);

    highp vec4  st      = vec4(wPos, time) / resolution;
    highp float flicker = simplexNoise(st);
    return flicker * amplitude + 1.0;
}

/* Compute the torch light. The argument "torchLevel" should be the
 * torch light level [0, 1].
 */
vec3 torchLight(float torchLevel, float sunLevel, float daylight, highp float flickerFactor) {
    const float baseIntensity = 180.0;
    const float decay         = 5.0;

    if (torchLevel > 0.0) {
        float intensity = baseIntensity * pow(torchLevel, decay) * flickerFactor;

        /* Reduce the effect of the torch light on areas lit by the
         * sunlight. Theoretically we shouldn't need to do this and
         * instead can use much more intense light for the sun, but we
         * haven't found a good tone mapping curve for that.
         */
        intensity *= mix(1.0, 0.1, smoothstep(0.65, 0.875, sunLevel * daylight));

        return torchlightColor * intensity;
    }
    else {
        return vec3(0);
    }
}

/* Compute the emissive light for light source objects.
 */
vec3 emissiveLight(highp float flickerFactor) {
    /* The game doesn't tell us what kind of light source it is, so we
     * assume it's a torch. */
    const vec3  lightColor = torchlightColor;
    const float intensity  = 60.0;

    return lightColor * intensity * flickerFactor;
}

/* Compute the sunlight based on the terrain-dependent sunlight level
 * [0,1] and the time-dependent daylight level "daylight" [0,1]. The
 * sunlight is yellow-ish red. The sunlight comes from the sun which
 * behaves like a directional light.
 */
vec3 sunlight(float sunLevel, float daylight) {
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

        return sunlightColor(daylight) * intensity;
    }
    else {
        return vec3(0);
    }
}

/* Compute the skylight based on the terrain-dependent sunlight level
 * [0,1] and the time-dependent daylight level "daylight" [0,1]. The
 * skylight is blue-ish white. The skylight comes from the sky which
 * behaves like an ambient light but is affected by occlusion.
 */
vec3 skylight(float sunLevel, float daylight) {
    const float baseIntensity = 30.0;

    float intensity = baseIntensity * sunLevel * daylight;
    if (intensity > 0.0) {
        return skylightColor * intensity;
    }
    else {
        return vec3(0);
    }
}

/* Compute the moonlight based on the time-dependent daylight level
 * [0, 1] and the terrain-dependent sunlight level [0, 1]. The
 * moonlight behaves like sunlight but is blue-ish white.
 */
vec3 moonlight(float sunLevel, float daylight) {
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

        return moonlightColor * intensity;
    }
    else {
        return vec3(0);
    }
}

/* Compute the specular light based on the surface normal and view
 * position.
 */
vec3 specularLight(
    float fresnel, float shininess, vec3 incomingDirLight, vec3 incomingUndirLight,
    highp vec3 worldPos, highp vec3 viewPos, highp vec3 normal) {

    vec3 incomingLight = incomingDirLight + incomingUndirLight;
    vec3 dirLightRatio = incomingDirLight / (incomingLight + vec3(0.001));

    /* The game doesn't tell us where the sun or the moon is, which is
     * so unfortunate. We have to assume they are always at some fixed
     * point. */
    const highp vec3 lightDir = normalize(vec3(-2.5, 5.5, 1.0));

    /* The intensity of the specular light is determined with the
     * angle between the Blinn-Phong half vector and the normal. See
     * https://seblagarde.wordpress.com/2011/08/17/hello-world/
     */
    highp vec3  viewDir   = -normalize(worldPos - viewPos);
    highp vec3  halfDir   = normalize(viewDir + lightDir);
    highp float incident  = max(0.0, dot(lightDir, halfDir));
    highp float reflAngle = max(0.0, dot(halfDir, normal));
    highp float dotNL     = max(0.0, dot(normal, lightDir));
    highp float reflCoeff = fresnel + (1.0 - fresnel) * pow(1.0 - incident, 5.0);
    highp vec3  specular  = incomingLight * 2.0 * pow(reflAngle, shininess) * reflCoeff * dotNL;

    highp float viewAngle = max(0.0, dot(normal, viewDir));
    highp float viewCoeff = fresnel + (1.0 - fresnel) * pow(1.0 - viewAngle, 5.0);
    return specular * dirLightRatio +     // Reflected directional light
        viewCoeff * incomingLight * 0.03; // Reflected undirectional light
}

#endif
