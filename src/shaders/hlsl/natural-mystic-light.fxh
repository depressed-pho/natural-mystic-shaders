// -*- hlsl -*-
#if !defined(NATURAL_MYSTIC_LIGH_FXH_INCLUDED)
#define NATURAL_MYSTIC_LIGH_FXH_INCLUDED 1

#include "natural-mystic-color.fxh"
#include "natural-mystic-noise.fxh"

/* Light color constants. Should be private to this file.
 */
static const float3 torchlightColor = float3(1.0, 0.66, 0.28);
static const float3 skylightColor   = float3(0.8392, 0.9098, 0.9961);
static const float3 moonlightColor  = float3(112.0, 135.5, 255.0)/255.0;

/* Calculate the color of sunlight based on the time-dependent
 * daylight level "daylight" [0, 1]. The color of sunlight changes
 * depending on the daylight level to express dusk and dawn.
 */
float3 sunlightColor(float daylight) {
    static const float3 setColor = float3(1.0, 0.3569, 0.0196);
    static const float3 dayColor = float3(1.0, 0.8706, 0.8039);

    return lerp(setColor, dayColor, smoothstep(0.45, 1.0, daylight));
}

/* Calculate the color of the ambient light based solely on the fog
 * color.
 */
float3 ambientLightColor(float4 fogColor) {
    return brighten(fogColor.rgb);
}

/* Calculate the color of the ambient light based on the
 * terrain-dependent sunlight level, and the time-dependent daylight
 * level. The level of ambient light is not dependent on the terrain
 * but the color is.
 */
float3 ambientLightColor(float sunLevel, float daylight) {
    /* The daylight color is a lerpture of sunlight and skylight. */
    float3 daylightColor = lerp(skylightColor, sunlightColor(daylight), 0.625);

    /* The influence of the sun and the moon depends on the daylight
     * level. */
    float3 outsideColor = lerp(moonlightColor, daylightColor, daylight);

    /* In caves the torch light is the only possible light source but
     * on the ground the sun or the moon is the most influential. */
    return brighten(lerp(torchlightColor, outsideColor, sunLevel));
}

/* Compute the ambient light to be accumulated to a fragment. Without
 * this filter, objects getting no light will be rendered in complete
 * darkness, which isn't how the reality works.
 */
float3 ambientLight(float3 lightColor, float intensity) {
    return lightColor * intensity;
}

/* Calculate the torch light flickering factor [0, 2] based on the
 * world coordinates and the in-game time.
 */
float torchLightFlicker(float3 wPos, float time) {
    /* The flicker factor is solely determined by the coords and the
     * time. Ideally it should be separately computed for each light
     * source in the scene, but we can't do it because shaders don't
     * have access to such information. We instead generate a
     * 4-dimensional simplex noise and slice it by time.
     */
    static const float amplitude  = 0.40;
    static const float4  resolution = float4(12.0, 12.0, 12.0, 0.8);

    float4  st      = float4(wPos, time) / resolution;
    float flicker = simplexNoise(st);
    return flicker * amplitude + 1.0;
}

/* Compute the torch light. The argument "torchLevel" should be the
 * torch light level [0, 1].
 */
float3 torchLight(float torchLevel, float sunLevel, float daylight, float flickerFactor) {
    static const float baseIntensity = 180.0;
    static const float decay         = 5.0;

    if (torchLevel > 0.0) {
        float intensity = baseIntensity * pow(torchLevel, decay) * flickerFactor;

        /* Reduce the effect of the torch light on areas lit by the
         * sunlight. Theoretically we shouldn't need to do this and
         * instead can use much more intense light for the sun, but we
         * haven't found a good tone mapping curve for that.
         */
        intensity *= lerp(1.0, 0.1, smoothstep(0.65, 0.875, sunLevel * daylight));

        return torchlightColor * intensity;
    }
    else {
        return 0.0;
    }
}

/* Compute the emissive light for light source objects.
 */
float3 emissiveLight(float flickerFactor) {
    /* The game doesn't tell us what kind of light source it is, so we
     * assume it's a torch. */
    static const float3  lightColor = torchlightColor;
    static const float intensity  = 60.0;

    return lightColor * intensity * flickerFactor;
}

/* Compute the sunlight based on the terrain-dependent sunlight level
 * [0,1] and the time-dependent daylight level "daylight" [0,1]. The
 * sunlight is yellow-ish red. The sunlight comes from the sun which
 * behaves like a directional light.
 */
float3 sunlight(float sunLevel, float daylight) {
    static const float baseIntensity = 50.0;
    static const float shadowFactor  = 0.01;  // [0, 1]
    static const float shadowBorder  = 0.87;  // [0, 1]
    static const float shadowBlur    = 0.003; // The higher the more blur.

    float intensity = baseIntensity * sunLevel * daylight;
    if (intensity > 0.0) {
        /* Shadows reduce the amount of sunlight. The reason why we
         * don't remove it entirely is that a shadowed area near a lit
         * area will receive higher amount of scattered light. If it
         * were completely occluded the sunlight level won't be
         * non-zero. */
        intensity *= lerp(
            shadowFactor, 1.0,
            smoothstep(shadowBorder - shadowBlur, shadowBorder + shadowBlur, sunLevel));

        return sunlightColor(daylight) * intensity;
    }
    else {
        return 0.0;
    }
}

/* Compute the skylight based on the terrain-dependent sunlight level
 * [0,1] and the time-dependent daylight level "daylight" [0,1]. The
 * skylight is blue-ish white. The skylight comes from the sky which
 * behaves like an ambient light but is affected by occlusion.
 */
float3 skylight(float sunLevel, float daylight) {
    static const float baseIntensity = 30.0;

    float intensity = baseIntensity * sunLevel * daylight;
    if (intensity > 0.0) {
        return skylightColor * intensity;
    }
    else {
        return 0.0;
    }
}

/* Compute the moonlight based on the time-dependent daylight level
 * [0, 1] and the terrain-dependent sunlight level [0, 1]. The
 * moonlight behaves like sunlight but is blue-ish white.
 */
float3 moonlight(float sunLevel, float daylight) {
    static const float baseIntensity = 10.0;
    static const float shadowFactor  = 0.20;  // [0, 1]
    static const float shadowBorder  = 0.87;  // [0, 1]
    static const float shadowBlur    = 0.003; // The higher the more blur.

    float intensity = baseIntensity * sunLevel * (1.0 - daylight);
    if (intensity > 0.0) {
        /* Shadows reduce the amount of moonlight. */
        intensity *= lerp(
            shadowFactor, 1.0,
            smoothstep(shadowBorder - shadowBlur, shadowBorder + shadowBlur, sunLevel));

        return moonlightColor * intensity;
    }
    else {
        return 0.0;
    }
}

/* Compute the specular light based on the surface normal and view
 * position.
 */
float3 specularLight(
    float fresnel, float shininess, float3 incomingDirLight, float3 incomingUndirLight,
    float3 worldPos, float3 normal) {

    float3 incomingLight = incomingDirLight + incomingUndirLight;
    float3 dirLightRatio = incomingDirLight / (incomingLight + 0.001);

    /* The game doesn't tell us where the sun or the moon is, which is
     * so unfortunate. We have to assume they are always at some fixed
     * point. */
    static const float3 lightDir = normalize(float3(-2.5, 5.5, 1.0));

    /* The intensity of the specular light is determined with the
     * angle between the Blinn-Phong half floattor and the normal. See
     * https://seblagarde.wordpress.com/2011/08/17/hello-world/
     */
    float3  viewDir   = -normalize(worldPos);
    float3  halfDir   = normalize(viewDir + lightDir);
    float incident  = max(0.0, dot(lightDir, halfDir));
    float reflAngle = max(0.0, dot(halfDir, normal));
    float dotNL     = max(0.0, dot(normal, lightDir));
    float reflCoeff = fresnel + (1.0 - fresnel) * pow(1.0 - incident, 5.0);
    float3  specular  = incomingLight * 2.0 * pow(reflAngle, shininess) * reflCoeff * dotNL;

    float viewAngle = max(0.0, dot(normal, viewDir));
    float viewCoeff = fresnel + (1.0 - fresnel) * pow(1.0 - viewAngle, 5.0);
    return specular * dirLightRatio +     // Reflected directional light
        viewCoeff * incomingLight * 0.03; // Reflected undirectional light
}

#endif /* NATURAL_MYSTIC_LIGH_FXH_INCLUDED */
