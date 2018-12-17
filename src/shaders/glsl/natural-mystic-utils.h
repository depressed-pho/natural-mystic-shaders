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
