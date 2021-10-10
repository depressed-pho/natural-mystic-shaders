// -*- hlsl -*-
#if !defined(NATURAL_MYSTIC_COLOR_FXH_INCLUDED)
#define NATURAL_MYSTIC_COLOR_FXH_INCLUDED 1

/* Calculate the luma of a color in the linear RGB color space. */
float rgb2luma(float3 color) {
    return dot(color, float3(0.22, 0.707, 0.071));
}

/* Desaturate a color in the linear RGB color space. The parameter
 * "degree" should be in [0,1] where 0 being no desaturation and 1
 * being full desaturation (completely gray). Note that the result of
 * the function is usually to be multiplied by the color of the
 * ambient light, or otherwise a violation of the law of conservation
 * of energy will happen (#30).
 */
float3 desaturate(float3 baseColor, float degree) {
    float luma = rgb2luma(baseColor);
    return lerp(baseColor, luma, degree);
}

/* Convert linear RGB to HSV. The x component of the result will be
 * the hue [0, 1], y will be the saturation, and z will be the
 * value. See http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
 */
float3 rgb2hsv(float3 c) {
    static const float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    float4 p = c.g < c.b ? float4(c.bg, K.wz) : float4(c.gb, K.xy);
    float4 q = c.r < p.x ? float4(p.xyw, c.r) : float4(c.r, p.yzx);

    float d = q.x - min(q.w, q.y);
    static const float e = 1.0e-10;
    return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

/* Convert HSV to linear RGB. See
 * http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
 */
float3 hsv2rgb(float3 c) {
    static const float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * lerp(K.xxx, saturate(p - K.xxx), c.y);
}

/* Calculate the color of the ambient light based on some color,
 * usually the fog color, by normalizing the RGB components so at
 * least one component becomes 1.0.
 */
float3 brighten(float3 color) {
    float rgbMax = max(color.r, max(color.g, color.b));
    float delta  = 1.0 - rgbMax;
    return color + delta;
}

/* Apply Uncharted 2 tone mapping to the original fragment "frag".
 * See: http://filmicworlds.com/blog/filmic-tonemapping-operators/
 */
float3 uncharted2ToneMap_(float3 x) {
    static const float A = 0.015; // Shoulder strength
    static const float B = 0.50; // Linear strength
    static const float C = 0.10; // Linear angle
    static const float D = 0.010; // Toe strength
    static const float E = 0.02; // Toe numerator
    static const float F = 0.30; // Toe denominator

    return ((x * (A * x + C * B) + D * E) / (x * (A * x + B) + D * F)) - E / F;
}
float3 uncharted2ToneMap(float3 frag, float whiteLevel, float exposureBias) {
    float3 curr = uncharted2ToneMap_(exposureBias * frag);
    float3 whiteScale = 1.0 / uncharted2ToneMap_(whiteLevel);
    float3 color = curr * whiteScale;

    return saturate(color);
}

/* Apply ACES filmic tone mapping to the original fragment "x". See:
 * https://knarkowicz.wordpress.com/2016/01/06/aces-filmic-tone-mapping-curve/
 * [Currently unused]
 */
float3 acesFilmicToneMap(float3 x) {
    static const float a = 2.51;
    static const float b = 0.03;
    static const float c = 2.43;
    static const float d = 0.59;
    static const float e = 0.14;

    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

/* Apply a contrast filter on some LDR linear RGB color. The contrast
 * must be in [0, 2]. Note that this function modifies both the
 * saturation and the luminance, which means if you are to decrease
 * the contrast you should multiply the result with the color of the
 * ambient light, or otherwise you'll get gray when it's
 * inappropriate.
 */
float3 contrastFilter(float3 color, float contrast) {
    float t = 0.5 - contrast * 0.5;
    return saturate(color * contrast + t);
}

/* Apply a contrast filter on some LDR luminance. The contrast must be
 * in [0, 2].
 */
float contrastFilter(float lum, float contrast) {
    float t = 0.5 - contrast * 0.5;
    return saturate(lum * contrast + t);
}

/* Apply an HDR exposure filter to the original LDR fragment
 * "frag". The resulting image will be HDR, and need to be tone-mapped
 * back to LDR at the last stage. [Currently unused] */
float3 hdrExposure(float3 frag, float overExposure, float underExposure) {
    float3 overExposed   = frag / overExposure;
    float3 normalExposed = frag;
    float3 underExposed = frag * underExposure;

    return lerp(overExposed, underExposed, normalExposed);
}

#endif /* NATURAL_MYSTIC_COLOR_FXH_INCLUDED */
