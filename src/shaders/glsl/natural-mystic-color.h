// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_COLOR_H_INCLUDED)
#define NATURAL_MYSTIC_COLOR_H_INCLUDED 1

/* Calculate the luma of a color in the linear RGB color space. */
float rgb2luma(vec3 color) {
    return dot(color, vec3(0.22, 0.707, 0.071));
}

/* Desaturate a color in the linear RGB color space. The parameter
 * "degree" should be in [0,1] where 0 being no desaturation and 1
 * being full desaturation (completely gray). Note that the result of
 * the function is usually to be multiplied by the color of the
 * ambient light, or otherwise a violation of the law of conservation
 * of energy will happen (#30).
 */
vec3 desaturate(vec3 baseColor, float degree) {
    float luma = rgb2luma(baseColor);
    return mix(baseColor, vec3(luma), degree);
}

/* Convert linear RGB to HSV. The x component of the result will be
 * the hue [0, 1], y will be the saturation, and z will be the
 * value. See http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
 */
vec3 rgb2hsv(vec3 c) {
    const vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = c.g < c.b ? vec4(c.bg, K.wz) : vec4(c.gb, K.xy);
    vec4 q = c.r < p.x ? vec4(p.xyw, c.r) : vec4(c.r, p.yzx);

    float d = q.x - min(q.w, q.y);
    const float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

/* Convert HSV to linear RGB. See
 * http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
 */
vec3 hsv2rgb(vec3 c) {
    const vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

/* Calculate the color of the ambient light based on some color,
 * usually the fog color, by normalizing the RGB components so at
 * least one component becomes 1.0.
 */
vec3 brighten(vec3 color) {
    float rgbMax = max(color.r, max(color.g, color.b));
    float delta  = 1.0 - rgbMax;
    return color + delta;
}

/* Apply Uncharted 2 tone mapping to the original fragment "frag".
 * See: http://filmicworlds.com/blog/filmic-tonemapping-operators/
 */
vec3 uncharted2ToneMap_(vec3 x) {
    const float A = 0.015; // Shoulder strength
    const float B = 0.50; // Linear strength
    const float C = 0.10; // Linear angle
    const float D = 0.010; // Toe strength
    const float E = 0.02; // Toe numerator
    const float F = 0.30; // Toe denominator

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}
vec3 uncharted2ToneMap(vec3 frag, float whiteLevel, float exposureBias) {
    vec3 curr = uncharted2ToneMap_(exposureBias * frag);
    vec3 whiteScale = 1.0 / uncharted2ToneMap_(vec3(whiteLevel));
    vec3 color = curr * whiteScale;

    return clamp(color, 0.0, 1.0);
}

/* Apply ACES filmic tone mapping to the original fragment "x". See:
 * https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
 * [Currently unused]
 */
vec3 acesFilmicToneMap(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;

    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

/* Apply a contrast filter on some LDR linear RGB color. The contrast
 * must be in [0, 2]. Note that this function modifies both the
 * saturation and the luminance, which means if you are to decrease
 * the contrast you should multiply the result with the color of the
 * ambient light, or otherwise you'll get gray when it's
 * inappropriate.
 */
vec3 contrastFilter(vec3 color, float contrast) {
    float t = 0.5 - contrast * 0.5;
    return clamp(color * contrast + t, 0.0, 1.0);
}

/* Apply a contrast filter on some LDR luminance. The contrast must be
 * in [0, 2].
 */
float contrastFilter(float lum, float contrast) {
    float t = 0.5 - contrast * 0.5;
    return clamp(lum * contrast + t, 0.0, 1.0);
}

/* Apply an HDR exposure filter to the original LDR fragment
 * "frag". The resulting image will be HDR, and need to be tone-mapped
 * back to LDR at the last stage. [Currently unused] */
vec3 hdrExposure(vec3 frag, float overExposure, float underExposure) {
    vec3 overExposed   = frag / overExposure;
    vec3 normalExposed = frag;
    vec3 underExposed = frag * underExposure;

    return mix(overExposed, underExposed, normalExposed);
}

#endif /* NATURAL_MYSTIC_COLOR_H_INCLUDED */
