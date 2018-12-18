// -*- glsl -*-

/* Apply the ambient light on the original fragment "frag". Without
 * this filter, objects getting no light will be rendered in complete
 * darkness, which isn't how the reality works.
 */
vec3 applyAmbientLight(vec3 frag) {
    vec3 level = max(vec3(0.1), 0.5 - frag) * 1.2 + 1.0;
    return frag * level;
}

/* Calculate and apply a shadow on the original fragment "frag". The
 * argument "torchLevel" should be the torch light level [0, 1],
 * "sunLevel" should be the terrain-dependent sunlight level [0, 1],
 * and "daylight" should be the time-dependent daylight level.
 */
vec3 applyShadow(vec3 frag, float torchLevel, float sunLevel, float daylight) {
    const vec3 shadowColor = vec3(0.0);
    const float minShadow = 0.0;  // [0, 1]
    const float maxShadow = 0.45; // [0, 1]
    const float torchLightCutOff = 0.1; // [0, 1]

    /* The less the sunlight level is, the darker the fragment will
     * be. */
    float amount = mix(maxShadow, minShadow,
                       smoothstep(0.865, 0.875, sunLevel));

    /* The existence of torch light should negate the effect of
     * shadows. The higher the torch light level is, the less the
     * shadow affects the color. However, we also don't want torches
     * to completely drive away the shadow, since the intensity of the
     * torch light wouldn't be comparable to that of sunlight.
     */
    amount *= mix(1.0, torchLightCutOff,
                  smoothstep(0.5, 1.0, torchLevel));

    /* Lack of daylight should also negate the effect of shadows,
     * because sadly the light map passed by the upstream doesn't take
     * torches into account.
     */
    amount *= mix(0.2, 1.0, daylight);

    return mix(frag, shadowColor, amount);
}

/* Calculate and apply the torch color on the original fragment
 * "frag". The argument "torchLevel" should be the torch light level
 * [0, 1], "sunLevel" should be the terrain-dependent sunlight level
 * [0, 1], and "daylight" should be the time-dependent daylight level.
 */
vec3 applyTorchColor(vec3 frag, float torchLevel, float sunLevel, float daylight) {
    const vec3 torchColor = vec3(0.8, 0.3, -0.2);
    const float torchDecay = 0.55; // [0, 1]
    const float sunlightCutOff = 0.1; // [0, 1]

    /* The sunlight should prevent torches from affecting the color,
     * but we also have to take the daylight level into account.
     */
    float amount = max(0.0, torchLevel - torchDecay);
    amount *= mix(1.0, sunlightCutOff, smoothstep(0.65, 0.875, sunLevel) * daylight);

    return frag + torchColor * amount;
}

/* Apply the sunlight on a fragment "frag" based on the time-dependent
 * daylight level "daylight" [0,1]. The argument "sunLevel" should be
 * the terrain-dependent sunlight level [0,1]. The sunlight is
 * yellow-ish red.
 */
vec3 applySunlight(vec3 frag, float sunLevel, float daylight) {
    const vec3 sunColor = vec3(0.7, 0.3, 0.0);

    float amount = (0.5 - abs(0.5 - daylight)) * sunLevel;
    float sunset = (frag.r + frag.g + frag.b) / 1.5;

    return mix(frag, sunset * sunColor, amount);
}

/* Apply the skylight on a fragment "frag" based on the time-dependent
 * daylight level "daylight" [0,1]. The argument "sunLevel" should be
 * the terrain-dependent sunlight level [0,1]. The skylight is
 * blue-ish white.
 */
vec3 applySkylight(vec3 frag, float sunLevel, float daylight) {
    const vec3 skyColor = vec3(0.8, 0.8, 1.0);
    const float skyLevel = 0.07;

    float amount = sunLevel * skyLevel * daylight;

    return frag + skyColor * amount;
}

/* Apply the moonlight on a fragment "frag" based on the
 * time-dependent daylight level "daylight" [0,1]. The argument
 * "torchLevel" should be the torch light level [0,1], and the
 * argument "sunLevel" should be the terrain-dependent sunlight level
 * [0,1]. The moonlight is purple-ish white.
 */
vec3 applyMoonlight(vec3 frag, float torchLevel, float sunLevel, float daylight) {
    const vec3 moonColor = vec3(0.4, 0.7, 0.9);
    const float moonLevel = 0.95;

    /* The reason why we take into account the torch light level is
     * that the intensity of the torch light is far higher than that
     * of moonlight.
     */
    float amount = sunLevel * (1.0 - torchLevel) * moonLevel * (1.0 - daylight);
    float desaturated = dot(frag, vec3(0.22, 0.707, 0.071));

    return mix(frag, desaturated * moonColor, amount);
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
vec3 uncharted2ToneMap(vec3 frag, float exposureBias) {
    const float W = 11.2;

    vec3 curr = uncharted2ToneMap_(exposureBias * frag);
    vec3 whiteScale = 1.0 / uncharted2ToneMap_(vec3(W, W, W));
    vec3 color = curr * whiteScale;

    return color;
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

/* Apply a contrast filter to the original fragment "frag".
 */
vec3 contrastFilter(vec3 frag, float contrast) {
    return (frag - 0.5) * max(contrast, 0.0) + 0.5;
}

/* Apply an HDR exposure filter to the original LDR fragment
 * "frag". The resulting image will be HDR, and need to be tone-mapped
 * back to LDR at the last stage. */
vec3 hdrExposure(vec3 frag, float overExposure, float underExposure) {
    vec3 overExposed   = frag / overExposure;
    vec3 normalExposed = frag;
    vec3 underExposed = frag * underExposure;

    return mix(overExposed, underExposed, normalExposed);
}
