// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_FOG_H_INCLUDED)
#define NATURAL_MYSTIC_FOG_H_INCLUDED 1

/* Compute the density [0, 1] of linear fog based on a near/far
 * control and a camera distance. It is the same as what vanilla does,
 * and is the most cheap one. */
float linearFog(vec2 control, float dist) {
    float density = (dist - control.x) / (control.y - control.x);
    return clamp(density, 0.0, 1.0);
}

float exponentialFog(vec2 control, float dist) {
    /* Determine the base density so the final result will be maxed
     * out to 1 at the fog far, i.e. the y component of the fog
     * control.
     *
     *   1 / e^(far * base) <= r (for some very small positive r),
     *   r * e^(far * base) >= 1,
     *   e^(far * base) >= 1/r,
     *   far * base >= log 1/r, therefore
     *   base >= (log 1/r) / far
     */
    float base = log(1.0/0.03) / (control.y - control.x);
    dist = max(0.0, dist - control.x);

    float fogFactor = 1.0 / exp(dist * base);
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    return 1.0 - fogFactor;
}

/* Compute the density [0, 1] of exponential squared fog based on a
 * near/far control and a camera distance. This function produces
 * better fogs than those of vanilla (#12). See:
 * http://in2gpu.com/2014/07/22/create-fog-shader/
 */
float exponentialSquaredFog(vec2 control, float dist) {
    /* Determine the base density so the final result will be maxed
     * out to 1 at the fog far, i.e. the y component of the fog
     * control.
     *
     *   1 / e^((far * base)^2) <= r (for some very small positive r),
     *   r * e^((far * base)^2) >= 1,
     *   e^((far * base)^2) >= 1/r,
     *   (far * base)^2 >= log 1/r,
     *   far * base >= sqrt (log 1/r), therefore
     *   base >= (sqrt (log 1/r)) / far
     */
    float base = sqrt(log(1.0/0.015)) / (control.y - control.x);
    dist = max(0.0, dist - control.x);

    float fogFactor = 1.0 / exp(pow(dist * base, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    return 1.0 - fogFactor;
}

/* Compute the brightness [0, 1] of permanent fog as opposed to the
 * bad weather fog. Fogs can scatter any of incoming light rays
 * regardless of their source.
 */
float fogBrightness(float torchLevel, float sunLevel, float daylight) {
    const float scatter = 1.2;

    float brightness = max(torchLevel, sunLevel * daylight) * scatter;
    return clamp(brightness, 0.0, 1.0);
}

#endif /* defined(NATURAL_MYSTIC_FOG_H_INCLUDED) */
