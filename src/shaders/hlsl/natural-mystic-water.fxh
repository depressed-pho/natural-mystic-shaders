// -*- hlsl -*-
#if !defined(NATURAL_MYSTIC_WATER_FXH_INCLUDED)
#define NATURAL_MYSTIC_WATER_FXH_INCLUDED 1

/* Overview of our water system:
 *
 * In the vertex shader we compute geometric undulations of a base
 * mesh map with Gerstner waves, as well as their partial derivatives
 * to compute the surface normal. The opacity of the water is also
 * computed in the vertex shader based on the view angle. The
 * resulting normal is then passed to the fragment shader.
 *
 * In the fragment shader we generate smaller waves and sum up with
 * the geometric normal, to get the final normal. We use the normal to
 * compute the intensity of the specular light.
 *
 * See also: https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch01.html
 * And: https://hal.inria.fr/inria-00443630/file/article-1.pdf
 */

/* Compute a Gerstner wave. See comments in waterWaveGeometric() for
 * the meaning of parameters.
 */
float3 gerstnerWave(
    float3 wPos, float time, inout float3 normal,
    float Q, float numWaves, float Ai, float2 Di, float Li, float Si) {

    static const float wFactor = 9.80665 * 2.0 * 3.14159;

    float wi    = sqrt(wFactor / Li);
    float Qi    = Q / (wi * Ai * numWaves);
    float phi_i = Si * 2.0 / Li;

    float theta    = wi * dot(Di, wPos.xz) + phi_i * time;
    float cosTheta = cos(theta);
    float sinTheta = sin(theta);
    wPos.xz += Di * Qi * Ai * cosTheta;
    wPos.y  +=           Ai * sinTheta;

    float wiAi = wi * Ai;
    normal.xz -= wiAi * Di * cosTheta;
    normal.y  -= wiAi * Qi * sinTheta;

    return wPos;
}

/* Similar to gerstnerWave() but this only computes the normal floattor.
 */
float3 gerstnerWaveN(
    float3 wPos, float time, float3 normal,
    float Q, float numWaves, float Ai, float2 Di, float Li, float Si) {

    static const float wFactor = 9.80665 * 2.0 * 3.14159;

    float wi    = sqrt(wFactor / Li);
    float Qi    = Q / (wi * Ai * numWaves);
    float phi_i = Si * 2.0 / Li;

    float theta = wi * dot(Di, wPos.xz) + phi_i * time;
    float wiAi  = wi * Ai;
    normal.xz -= wiAi * Di * cos(theta);
    normal.y  -= wiAi * Qi * sin(theta);

    return normal;
}

/* Translate an angle in degrees into a normalized direction floattor,
 * hoping drivers will optimize it out.
 */
float2 deg2dir(float deg) {
    float rad = radians(deg);
    return float2(cos(rad), sin(rad));
}

float3 waterWaveGeometric(float3 wPos, float time, out float3 normal) {
    /* The Gerstner wave function is:
     *
     *              [ x + Σ(Q_i A_i * D_i.x * cos(w_i D_i · (x, y) + φ_i t)), ]
     * P(x, y, t) = | y + Σ(Q_i A_i * D_i.y * cos(w_i D_i · (x, y) + φ_i t)), |
     *              [     Σ(    A_i         * sin(w_i D_i · (x, y) + φ_i t))  ]
     *
     * where
     *
     *   Q_i: The steepness of the i-th wave. Q_i = 0 means that the
     *        wave i is plainly a sine, while Q_i = 1/(w_i A_i) gives
     *        a sharp crest. To ease setting the parameters, we leave
     *        the specification of Q as a steepness parameter [0, 1]
     *        and use Q_i = Q/(w_i A_i * numWaves).
     *
     *   A_i: The amplitude of the i-th wave.
     *
     *   D_i: The horizontal floattor of the i-th wave, perpendicular to
     *        the wave front along which the crest travels.
     *
     *   w_i: The frequency of the i-th wave. For wavelength L_i, w_i
     *        = sqrt(g * 2π/L_i) where g is the gravitational
     *        constant 9.80665.
     *
     *   φ_i: The phase constant of the i-th wave. For speed S_i,
     *        φ_i = S_i * 2/L_i.
     *
     * And the surface normal will be:
     *
     *              [    -Σ(D_i.x * w_i A_i * cos(w_i D_i · (x, y) + φ_i t)), ]
     * N(x, y, t) = |    -Σ(D_i.y * w_i A_i * cos(w_i D_i · (x, y) + φ_i t)), |
     *              [ 1 - Σ(Q_i   * w_i A_i * sin(w_i D_i · (x, y) + φ_i t))  ]
     */
    static const float Q        = 0.45;
    static const float numWaves = float(4);

    normal = float3(0.0, 1.0, 0.0);
    wPos   = gerstnerWave(wPos, time, normal, Q, numWaves, 0.08, deg2dir( 90.0), 16.0, 7.0);
    wPos   = gerstnerWave(wPos, time, normal, Q, numWaves, 0.08, deg2dir(260.0), 15.0, 8.0);
    wPos   = gerstnerWave(wPos, time, normal, Q, numWaves, 0.05, deg2dir( 70.0),  8.0, 13.0);
    wPos   = gerstnerWave(wPos, time, normal, Q, numWaves, 0.02, deg2dir(200.0),  7.0, 14.0);

    return wPos;
}

float3 waterWaveNormal(float3 wPos, float time, float3 normal) {
    const float Q        = 0.45;
    const float numWaves = float(3);

    normal = gerstnerWaveN(wPos, time, normal, Q, numWaves, 0.0058, deg2dir( 85.0), 0.75, 1.0);
    normal = gerstnerWaveN(wPos, time, normal, Q, numWaves, 0.0058, deg2dir(255.0), 0.725, 2.0);
    normal = gerstnerWaveN(wPos, time, normal, Q, numWaves, 0.0045, deg2dir( 65.0), 0.7, 2.0);

    return normal;
}

/* Compute the specular light on a water surface, and the opacity at
 * the same time. The .a component of the result should be used as an
 * absolute, not relative opacity.
 */
float4 waterSpecularLight(
    float baseOpacity, float3 incomingDirLight, float3 incomingUndirLight,
    float3 worldPos, float time, float3 normal) {

    /* Compute the contribution of directional light (i.e. the sun and
     * the moon) to the entire incoming light. */
    float3 incomingLight = incomingDirLight + incomingUndirLight;
    float3 dirLightRatio = incomingDirLight / (incomingLight + 0.001);

    /* The game doesn't tell us where the sun or the moon is, which is
     * so unfortunate. We have to assume they are always at some fixed
     * point. */
    static const float3 lightDir = normalize(float3(-2.5, 2.5, 0.0));

    /* The intensity of the specular light is determined with the
     * angle between the Blinn-Phong half floattor and the normal. See:
     * https://en.wikibooks.org/wiki/GLSL_Programming/GLUT/Specular_Highlights
     * https://www.gamedev.net/forums/topic/625142-blinn-phong-with-fresnel-effect/
     * http://filmicworlds.com/blog/everything-has-fresnel/
     * https://hal.inria.fr/inria-00443630/file/article-1.pdf
     */
    float3  viewDir      = -normalize(worldPos);
    static const float shininess    = 80.0;

    /* Compute the Fresnel term between the view floattor and the half
     * floattor. When the angle is wide the water surface behaves more
     * like a mirror than air. Note that constant "fresnel" is:
     *
     * [ iorIn - iorOut ]2
     * | -------------- |   where iorIn = 1.0 (the refraction index
     * [ iorIn + iorOut ]   of air) and iorOut = 1.33 (water).
     */
    static const float fresnel      = 0.02;
    float3  halfDir      = normalize(viewDir + lightDir);
    float incident     = max(0.0, dot(viewDir, halfDir)); // Cosine of the angle.
    float reflAngle    = max(0.0, dot(halfDir, normal));
    float reflCoeff    = fresnel + (1.0 - fresnel) * pow(1.0 - incident, 5.0);
    float specCoeff    = pow(reflAngle, shininess) * reflCoeff;
    float3  specular     = incomingLight * 180.0 * specCoeff;

    /* Compute the opacity of water. In real life when a light ray
     * hits a surface of water, some part of it will reflect away, and
     * the other part will refract and bounces back from the bottom
     * (Fresnel effect). This means the opacity is dependent on the
     * depth of water body. But we can't actually do it with just a
     * single render pass so we have to somehow approximate it. Here
     * we compute the Fresnel term between the normal and the view
     * floattor, and consider that the surface reflects all the possible
     * incoming light rays (including ambient light and skylight), and
     * when that happens the water is opaque. This is of course a
     * crude hack and isn't based on the real optics. */
    float viewAngle    = max(0.0, dot(normal, viewDir));
    float opacCoeff    = fresnel + (1.0 - fresnel) * pow(1.0 - viewAngle, 5.0);
    float opacity      = lerp(baseOpacity, min(1.0, baseOpacity * 8.0), opacCoeff);

    float sharpOpac    = smoothstep(0.1, 0.2, opacCoeff);
    return float4(
        specular * dirLightRatio * sharpOpac + // Reflected directional light
        opacCoeff * incomingLight * 0.15,      // Reflected undirectional light
        lerp(opacity, 1.0, specCoeff));
}

#endif /* !defined(NATURAL_MYSTIC_WATER_FXH_INCLUDED) */
