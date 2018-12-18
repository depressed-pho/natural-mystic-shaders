// -*- glsl -*-

/* Calculate and apply a shadow on the original fragment "frag". The
 * argument "torch" should be the torch light level [0, 1] and "sun"
 * should be the sunlight level [0, 1].
 */
vec3 applyShadow(vec3 frag, float torch, float sun) {
    const vec3 shadowColor = vec3(0.0);
    const float minShadowLevel = 0.0;  // [0, 1]
    const float maxShadowLevel = 0.45; // [0, 1]
    const float torchLightCutOff = 0.1; // [0, 1]

    /* The less the sunlight level is, the darker the fragment will
     * be. */
    float shadowLevel = mix(maxShadowLevel, minShadowLevel,
                            smoothstep(0.865, 0.875, sun));

    /* The existence of torch light should negate the effect of
     * shadows. The higher the torch light level is, the less the
     * shadow affects the color. However, we also don't want torches
     * to completely drive away the shadow, since the intensity of the
     * torch light wouldn't be comparable to that of sunlight.
     */
    shadowLevel *= mix(1.0, torchLightCutOff,
                       smoothstep(0.5, 1.0, torch));

    return mix(frag, shadowColor, shadowLevel);
}

/* Calculate and apply the torch color on the original fragment
 * "frag". The argument "torch" should be the torch light level [0, 1]
 * and "sun" should be the sunlight level [0, 1].
 */
vec3 applyTorchColor(vec3 frag, float torch, float sun) {
    const vec3 torchColor = vec3(0.8, 0.3, -0.2);
    const float torchIntensity = 0.6; // [0, 1]
    const float sunlightCutOff = 0.1; // [0, 1]

    /* The sunlight should prevent torches from affecting the color.
     */
    float torchLevel = max(0.0, torch - torchIntensity) *
        mix(1.0, sunlightCutOff, smoothstep(0.7, 0.9, sun));

    return min(frag + torchColor * torchLevel, 1.0);
}

/* Apply Uncharted 2 tone mapping to the original fragment "frag".
 * See: http://filmicworlds.com/blog/filmic-tonemapping-operators/
 */
vec3 uncharted2ToneMap_(vec3 x) {
    const float A = 0.15;
    const float B = 0.50;
    const float C = 0.10;
    const float D = 0.20;
    const float E = 0.02;
    const float F = 0.30;

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}
vec3 uncharted2ToneMap(vec3 frag) {
    const float W = 11.2;
    const float exposureBias = 2.0;
    const float gamma = 1.3;

    vec3 curr = uncharted2ToneMap_(exposureBias * frag);
    vec3 whiteScale = 1.0 / uncharted2ToneMap_(vec3(W, W, W));
    vec3 color = curr * whiteScale;

    return pow(color, vec3(1.0 / gamma));
}

/* Apply ACES filmic tone mapping to the original fragment "x".
 * See:
 * https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
 */
vec3 acesFilmicToneMap(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

/* Apply an HDR filter to the original fragment "frag". */
vec3 hdrFilter(vec3 frag, float contrast, float overExposure, float underExposure) {
    vec3 overExposed   = frag / overExposure;
    vec3 normalExposed = frag;
    vec3 underExposed = frag * underExposure;

    frag = mix(overExposed, underExposed, normalExposed);
    frag = (frag - 0.5) * max(contrast, 0.0) + 0.5;
    return frag;
}
