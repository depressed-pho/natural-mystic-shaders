// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_NOISE_H_INCLUDED)
#define NATURAL_MYSTIC_NOISE_H_INCLUDED 1

/* Generate a random scalar [0, 1) base on some scalar.
 */
highp float random(highp float st) {
    st += 128.0; // The function has a bad characteristic near 0.
    return fract(sin(st * 12.9898) * 43758.5453123);
}

/* Generate a random scalar [0, 1) based on some 2D vector. See
 * https://thebookofshaders.com/13/
 */
highp float random(highp vec2 st) {
    st += 128.0; // The function has a bad characteristic near (0, 0).
    return fract(
        sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
}

/* Generate a 1D perlin noise based on some scalar.
 */
highp float perlinNoise(highp float st) {
    highp float i = floor(st);
    highp float f = fract(st);

    // Two borders in an interval.
    highp float a = random(i);
    highp float b = random(i + 1.0);

    highp float u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u);
}

/* Generate a 2D perlin noise based on some 2D vector. Based on Morgan
 * McGuire @morgan3d https://www.shadertoy.com/view/4dS3Wd
 */
highp float perlinNoise(highp vec2 st) {
    highp vec2 i = floor(st);
    highp vec2 f = fract(st);

    // Four corners in 2D of a tile.
    highp float a = random(i);
    highp float b = random(i + vec2(1.0, 0.0));
    highp float c = random(i + vec2(0.0, 1.0));
    highp float d = random(i + vec2(1.0, 1.0));

    highp vec2 u = f * f * (3.0 - 2.0 * f);

    return mix(a, b, u.x) +
        (c - a) * u.y * (1.0 - u.x) +
        (d - b) * u.x * u.y;
}

/* Generate an fBM noise based on some 2D vector. See
 * https://thebookofshaders.com/13/
 */
highp float fBM(int octaves, highp vec2 st) {
    // Initial values
    highp float value = 0.0;
    highp float amplitude = 0.5;

    // Loop of octaves
    for (int i = 0; i < octaves; i++) {
        value += amplitude * perlinNoise(st);
        st *= 2.0;
        amplitude *= 0.5;
    }
    return value;
}

#endif /* NATURAL_MYSTIC_NOISE_H_INCLUDED */
