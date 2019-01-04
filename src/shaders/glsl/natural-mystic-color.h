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
 * the hue, y will be the saturation, and z will be the value. It does
 * not change the alpha. */
vec4 rgb2hsv(vec4 rgb) {
    highp float rgbMax = max(rgb.r, max(rgb.g, rgb.b));
    highp float rgbMin = min(rgb.r, min(rgb.g, rgb.b));
    highp float h      = 0.0;
    highp float s      = 0.0;
    highp float v      = rgbMax;
    highp float delta  = rgbMax - rgbMin;

    if (delta != 0.0) {
        if (rgbMax == rgb.r) {
            // Between yellow and magenta.
            h = (rgb.g - rgb.b) / delta;
        }
        else if (rgbMax == rgb.g) {
            // Between cyan and yellow;
            h = 2.0 + (rgb.b - rgb.r) / delta;
        }
        else {
            // Between magenta and syan.
            h = 4.0 + (rgb.r - rgb.g) / delta;
        }
    }
    h *= 60.0; // degree
    h  = h < 0.0 ? h + 360.0 : h;

    if (rgbMax != 0.0) {
        s = delta / rgbMax;
    }

    return vec4(h, s, v, rgb.a);
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

/* Compute the fog color based on a base fog color, and a camera
 * distance. The resulting color should be mixed with the light color
 * using the alpha of the result. This function produces better fogs
 * than those of vanilla (#12). [Currently unused]
 */
vec4 computeFogColor(vec3 baseFog, float dist) {
    // See: http://in2gpu.com/2014/07/22/create-fog-shader/
    const float density = 0.01;

    float fogFactor = 1.0 / exp(pow(dist * density, 2.0));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    return vec4(baseFog, 1.0 - fogFactor);
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

/* Apply a contrast filter on some LDR color.
 */
vec3 contrastFilter(vec3 color, float contrast) {
    return clamp((color - 0.5) * contrast + 0.5, 0.0, 1.0);
}

/* Apply a contrast filter on some LDR luminance.
 */
float contrastFilter(float lum, float contrast) {
    float t = 0.5 - contrast * 0.5;
    return clamp(lum * contrast + t, 0.0, 1.0);
    return clamp((lum - 0.5) * contrast + 0.5, 0.0, 1.0);
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
