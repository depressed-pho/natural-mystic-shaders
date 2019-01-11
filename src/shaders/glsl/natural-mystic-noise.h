// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_NOISE_H_INCLUDED)
#define NATURAL_MYSTIC_NOISE_H_INCLUDED 1

// See also: https://www.shadertoy.com/view/4djSRW
// Also https://briansharpe.wordpress.com/2011/10/01/gpu-texture-free-noise/
// Also https://github.com/stegu/webgl-noise/

/* Generate a random scalar [0, 1) based on some 2D vector. See
 * https://thebookofshaders.com/13/
 */
highp float random(highp vec2 st) {
    st += 128.0; // The function has a bad characteristic near (0, 0).
    return fract(
        sin(dot(st, vec2(12.9898, 4.1414))) * 43758.5453123);
}

/* Generate a 2D perlin noise. Based on Morgan McGuire @morgan3d
 * https://www.shadertoy.com/view/4dS3Wd
 */
highp float perlinNoise(highp vec2 st) {
    highp vec2 i = floor(st);
    highp vec2 f = fract(st);

    // Four corners in 2D of a tile.
    highp float a = random(i);
    highp float b = random(i + vec2(1.0, 0.0));
    highp float c = random(i + vec2(0.0, 1.0));
    highp float d = random(i + vec2(1.0, 1.0));

    // Simple 2D lerp using smoothstep envelope between the values.
    // return mix(mix(a, b, smoothstep(0.0, 1.0, f.x)),
    //            mix(c, d, smoothstep(0.0, 1.0, f.x)),
    //            smoothstep(0.0, 1.0, f.y));

    // Same code, with the clamps in smoothstep and common subexpressions
    // optimized away.
    highp vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(a, b, u.x) +
        (c - a) * u.y * (1.0 - u.x) +
        (d - b) * u.x * u.y;
}

/* Permutation in mod 289. */
highp vec4 permute289(highp vec4 x) {
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}

highp float permute289(highp float x) {
    return mod(((x * 34.0) + 1.0) * x, 289.0);
}

/* A 4D gradient function, used for generating noise. */
highp vec4 grad4(highp float j, highp vec4 ip) {
    const vec4 ones = vec4(1.0, 1.0, 1.0, -1.0);
    vec4 p, s;

    p.xyz  = floor(fract(j * ip.xyz) * 7.0) * ip.z - 1.0;
    p.w    = 1.5 - dot(abs(p.xyz), ones.xyz);
    p.xyz -= sign(p.xyz) * (p.w < 0.0 ? 1.0 : 0.0);

    return p;
}

/* 4D simplex noise [-1, 1], based on https://github.com/stegu/webgl-noise/
 */
highp float simplexNoise(highp vec4 v) {
    const highp vec4 C = vec4(
        0.138196601125011,    // (5 - sqrt(5))/20 = G4
        0.276393202250021,    // 2 * G4
        0.414589803375032,    // 3 * G4
        -0.447213595499958);  // -1 + 4 * G4
    const highp float F4 = 0.309016994374947451; // (sqrt(5) - 1) / 4

    // First corner
    highp vec4 i  = floor(v + dot(v, vec4(F4)));
    highp vec4 x0 = v -   i + dot(i, C.xxxx);

    // Other corners

    // Rank sorting originally contributed by Bill Licea-Kane, AMD
    // (formerly ATI)
    highp vec4 i0;
    highp vec3 isX  = step(x0.yzw, x0.xxx);
    highp vec3 isYZ = step(x0.zww, x0.yyz);
    i0.x    = dot(isX, vec3(1.0));
    i0.yzw  = 1.0 - isX;
    i0.y   += dot(isYZ.xy, vec2(1.0));
    i0.zw  += 1.0 - isYZ.xy;
    i0.z   += isYZ.z;
    i0.w   += 1.0 - isYZ.z;

    // i0 now contains the unique values 0, 1, 2, 3 in each channel
    highp vec4 i3 = clamp(i0      , 0.0, 1.0);
    highp vec4 i2 = clamp(i0 - 1.0, 0.0, 1.0);
    highp vec4 i1 = clamp(i0 - 2.0, 0.0, 1.0);

    //         x0 = x0 - 0.0 + 0.0 * C.xxxx
    //         x1 = x0 - i1  + 1.0 * C.xxxx
    //         x2 = x0 - i2  + 2.0 * C.xxxx
    //         x3 = x0 - i3  + 3.0 * C.xxxx
    //         x4 = x0 - 1.0 + 4.0 * C.xxxx
    highp vec4 x1 = x0 - i1  + C.xxxx;
    highp vec4 x2 = x0 - i2  + C.yyyy;
    highp vec4 x3 = x0 - i3  + C.zzzz;
    highp vec4 x4 = x0       + C.wwww;

    // Permutations
    i = mod(i, 289.0);
    highp float j0 =
        permute289(
            permute289(
                permute289(
                    permute289(i.w) + i.z
                    ) + i.y
                ) + i.x
            );
    highp vec4 j1 =
        permute289(
            permute289(
                permute289(
                    permute289(
                        i.w + vec4(i1.w, i2.w, i3.w, 1.0)
                        ) + i.z + vec4(i1.z, i2.z, i3.z, 1.0)
                    ) + i.y + vec4(i1.y, i2.y, i3.y, 1.0)
                ) + i.x + vec4(i1.x, i2.x, i3.x, 1.0)
            );

    // Gradients: 7x7x6 points over a cube, mapped onto a 4-cross
    // polytope 7*7*6 = 294, which is close to the ring size 17*17 =
    // 289.
    const highp vec4 ip = vec4(
        0.003401360544217687075, // 1/294
        0.020408163265306122449, // 1/49
        0.142857142857142857143, // 1/7
        0.0);

    highp vec4 p0 = normalize(grad4(j0  , ip));
    highp vec4 p1 = normalize(grad4(j1.x, ip));
    highp vec4 p2 = normalize(grad4(j1.y, ip));
    highp vec4 p3 = normalize(grad4(j1.z, ip));
    highp vec4 p4 = normalize(grad4(j1.w, ip));

    // Mix contributions from the five corners
    highp vec3 m0 = max(0.5 - vec3(dot(x0, x0), dot(x1, x1), dot(x2, x2)), 0.0);
    highp vec2 m1 = max(0.5 - vec2(dot(x3, x3), dot(x4, x4)             ), 0.0);
    m0 = m0 * m0;
    m1 = m1 * m1;
    return 49.0 *
        ( dot(m0 * m0, vec3(dot(p0, x0), dot(p1, x1), dot(p2, x2))) +
          dot(m1 * m1, vec2(dot(p3, x3), dot(p4, x4))) );
}

/* Generate a 2D fBM noise. See https://thebookofshaders.com/13/
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
