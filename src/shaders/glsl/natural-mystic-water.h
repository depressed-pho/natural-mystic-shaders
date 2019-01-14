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

highp vec3 gerstnerWave(
    highp vec3 wPos, highp float time, inout highp vec3 normal,
    float Q, float numWaves, float Ai, vec2 Di, float Li, float Si) {

    const float wFactor = 9.80665 * 2.0 * 3.14159;

    float wi    = sqrt(wFactor / Li);
    float Qi    = Q / (wi * Ai * numWaves);
    float phi_i = Si * 2.0 / Li;

    highp float phase = wi * dot(Di, wPos.xz) + phi_i * time;
    wPos.xz += Qi * Ai * Di * cos(phase);
    wPos.y  +=      Ai      * sin(phase);

    highp float wiAi = wi * Ai;
    normal.xz -= wiAi * Di * cos(phase);
    normal.y  -= wiAi * Qi * sin(phase);

    return wPos;
}

highp vec3 gerstnerWaveN(
    highp vec3 wPos, highp float time, highp vec3 normal,
    float Q, float numWaves, float Ai, vec2 Di, float Li, float Si) {

    const float wFactor = 9.80665 * 2.0 * 3.14159;

    float wi    = sqrt(wFactor / Li);
    float Qi    = Q / (wi * Ai * numWaves);
    float phi_i = Si * 2.0 / Li;

    highp float phase = wi * dot(Di, wPos.xz) + phi_i * time;
    highp float wiAi  = wi * Ai;
    normal.xz -= wiAi * Di * cos(phase);
    normal.y  -= wiAi * Qi * sin(phase);

    return normal;
}

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

    normal = gerstnerWaveN(wPos, time, normal, Q, numWaves, 0.0058, deg2dir( 85.0), 1.50, 1.0);
    normal = gerstnerWaveN(wPos, time, normal, Q, numWaves, 0.0058, deg2dir(255.0), 1.45, 2.0);
    normal = gerstnerWaveN(wPos, time, normal, Q, numWaves, 0.0045, deg2dir( 65.0), 1.40, 2.0);

    return normalize(normal);
}

vec4 waterColor(
    vec4 pigment, vec3 ambient,
    highp vec3 worldPos, highp vec3 relPos, highp float time, highp vec3 normal) {

    /* The game doesn't tell us where the sun or the moon is, which is
     * so unfortunate. We have to assume they are always near the
     * origin. */
    const highp vec3 sunMoonPos = vec3(-2.5, 1.0e10, 0);

    /* Oscillate the normal even more, but this time with much higher
     * frequencies. This is a kind of bump mapping.
     */
    normal = waterWaveNormal(worldPos + VIEW_POS, time, normal);

    /* The intensity of the specular light is determined with the
     * angle between the reflected sun light and the view vector. See
     * https://en.wikibooks.org/wiki/GLSL_Programming/GLUT/Specular_Highlights
     */
    highp vec3  viewPoint    = normalize(-relPos);
    highp vec3  reflectedSun = normalize(reflect(-sunMoonPos, normal));
    const float intensity    = 60.0; // FIXME: Caves and night. Also torches?
    const float shininess    = 2.0;
    highp float sunAngle     = max(0.0, dot(reflectedSun, viewPoint));
    highp vec3  specular     = ambient * intensity * pow(sunAngle, shininess);

    /* We want to know the angle between the normal of the water plane
     * (or anything rendered similarly to water) and the camera. The
     * value of viewAngle is actually cos θ, not in radian. */
    highp float viewAngle = abs(dot(normal, viewPoint)); // [0, 1]

    /* Use the angle to calculate the color of surface. The more steep
     * the angle is, the less intensity of the supecular light will
     * be. And we also use the angle to determine the opacity, that
     * is, when the angle is wide the water plane behaves more like a
     * mirror than air.
     */
    float opacity  = pigment.a;
    vec4 surfColor = vec4(specular, min(1.0, pigment.a * 8.0));
    vec4 nearColor = vec4(vec3(0), opacity);

    return mix(surfColor, nearColor, smoothstep(0.0, 0.8, viewAngle));
}

#endif /* !defined(NATURAL_MYSTIC_WATER_H_INCLUDED) */
