// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_WATER_H_INCLUDED)
#define NATURAL_MYSTIC_WATER_H_INCLUDED 1

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
 */

/* Compute a Gerstner wave. See comments in waterWaveGeometric() for
 * the meaning of parameters.
 */
highp vec3 gerstnerWave(
    highp vec3 wPos, highp float time, inout highp vec3 normal,
    float Q, float numWaves, float Ai, vec2 Di, float Li, float Si) {

    const float wFactor = 9.80665 * 2.0 * 3.14159;

    float wi    = sqrt(wFactor / Li);
    float Qi    = Q / (wi * Ai * numWaves);
    float phi_i = Si * 2.0 / Li;

    highp float theta    = wi * dot(Di, wPos.xz) + phi_i * time;
    highp float cosTheta = cos(theta);
    highp float sinTheta = sin(theta);
    wPos.xz += Di * Qi * Ai * cosTheta;
    wPos.y  +=           Ai * sinTheta;

    highp float wiAi = wi * Ai;
    normal.xz -= wiAi * Di * cosTheta;
    normal.y  -= wiAi * Qi * sinTheta;

    return wPos;
}

/* Similar to gerstnerWave() but this only computes the normal vector.
 */
highp vec3 gerstnerWaveN(
    highp vec3 wPos, highp float time, highp vec3 normal,
    float Q, float numWaves, float Ai, vec2 Di, float Li, float Si) {

    const float wFactor = 9.80665 * 2.0 * 3.14159;

    float wi    = sqrt(wFactor / Li);
    float Qi    = Q / (wi * Ai * numWaves);
    float phi_i = Si * 2.0 / Li;

    highp float theta = wi * dot(Di, wPos.xz) + phi_i * time;
    highp float wiAi  = wi * Ai;
    normal.xz -= wiAi * Di * cos(theta);
    normal.y  -= wiAi * Qi * sin(theta);

    return normal;
}

/* Translate an angle in degrees into a normalized direction vector,
 * hoping drivers will optimize it out.
 */
vec2 deg2dir(float deg) {
    float rad = radians(deg);
    return vec2(cos(rad), sin(rad));
}

highp vec3 waterWaveGeometric(highp vec3 wPos, highp float time, out highp vec3 normal) {
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
     *   D_i: The horizontal vector of the i-th wave, perpendicular to
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
    const float Q        = 0.45;
    const float numWaves = float(4);

    normal = vec3(0.0, 1.0, 0.0);
    wPos   = gerstnerWave(wPos, time, normal, Q, numWaves, 0.08, deg2dir( 90.0), 16.0, 7.0);
    wPos   = gerstnerWave(wPos, time, normal, Q, numWaves, 0.08, deg2dir(260.0), 15.0, 8.0);
    wPos   = gerstnerWave(wPos, time, normal, Q, numWaves, 0.05, deg2dir( 70.0),  8.0, 13.0);
    wPos   = gerstnerWave(wPos, time, normal, Q, numWaves, 0.02, deg2dir(200.0),  7.0, 14.0);

    return wPos;
}

highp vec3 waterWaveNormal(highp vec3 wPos, highp float time, highp vec3 normal) {
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
vec4 waterSpecularLight(
    float baseOpacity, vec3 incomingDirLight, vec3 incomingUndirLight,
    highp vec3 worldPos, highp vec3 viewPos, highp float time, highp vec3 normal) {

    /* Compute the contribution of directional light (i.e. the sun and
     * the moon) to the entire incoming light. */
    vec3 incomingLight = incomingDirLight + incomingUndirLight;
    vec3 dirLightRatio = incomingDirLight / (incomingLight + vec3(0.001));

    /* The game doesn't tell us where the sun or the moon is, which is
     * so unfortunate. We have to assume they are always at some fixed
     * point. */
    const highp vec3 lightDir = normalize(vec3(-2.5, 2.5, 0.0));

    /* The intensity of the specular light is determined with the
     * angle between the Blinn-Phong half vector and the normal. See:
     * https://en.wikibooks.org/wiki/GLSL_Programming/GLUT/Specular_Highlights
     * https://www.gamedev.net/forums/topic/625142-blinn-phong-with-fresnel-effect/
     * http://filmicworlds.com/blog/everything-has-fresnel/
     * https://hal.inria.fr/inria-00443630/file/article-1.pdf
     */
    highp vec3  viewDir      = -normalize(worldPos - viewPos);
    const float shininess    = 80.0;

    /* Compute the Fresnel term between the view vector and the half
     * vector. When the angle is wide the water surface behaves more
     * like a mirror than air. Note that constant "fresnel" is:
     *
     * [ iorIn - iorOut ]2
     * | -------------- |   where iorIn = 1.0 (the refraction index
     * [ iorIn + iorOut ]   of air) and iorOut = 1.33 (water).
     */
    const float fresnel      = 0.02;
    highp vec3  halfDir      = normalize(viewDir + lightDir);
    highp float incident     = max(0.0, dot(viewDir, halfDir)); // Cosine of the angle.
    highp float reflAngle    = max(0.0, dot(halfDir, normal));
    highp float reflCoeff    = fresnel + (1.0 - fresnel) * pow(1.0 - incident, 5.0);
    highp float specCoeff    = pow(reflAngle, shininess) * reflCoeff;
    highp vec3  specular     = incomingLight * 180.0 * specCoeff;

    /* Compute the opacity of water. In real life when a light ray
     * hits a surface of water, some part of it will reflect away, and
     * the other part will refract and bounces back from the bottom
     * (Fresnel effect). This means the opacity is dependent on the
     * depth of water body. But we can't actually do it with just a
     * single render pass so we have to somehow approximate it. Here
     * we compute the Fresnel term between the normal and the view
     * vector, and consider that the surface reflects all the possible
     * incoming light rays (including ambient light and skylight), and
     * when that happens the water is opaque. This is of course a
     * crude hack and isn't based on the real optics. */
    highp float viewAngle    = max(0.0, dot(normal, viewDir));
    highp float opacCoeff    = fresnel + (1.0 - fresnel) * pow(1.0 - viewAngle, 5.0);
    highp float opacity      = mix(baseOpacity, min(1.0, baseOpacity * 8.0), opacCoeff);

    highp float sharpOpac    = smoothstep(0.1, 0.2, opacCoeff);
    return vec4(
        specular * dirLightRatio * sharpOpac + // Reflected directional light
        opacCoeff * incomingLight * 0.15,      // Reflected undirectional light
        mix(opacity, 1.0, specCoeff));
}

#endif /* !defined(NATURAL_MYSTIC_WATER_H_INCLUDED) */
