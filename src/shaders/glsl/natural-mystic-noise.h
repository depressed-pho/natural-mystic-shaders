// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_NOISE_H_INCLUDED)
#define NATURAL_MYSTIC_NOISE_H_INCLUDED 1

// See also: https://www.shadertoy.com/view/4djSRW
// Also https://briansharpe.wordpress.com/2011/10/01/gpu-texture-free-noise/
// Also https://github.com/stegu/webgl-noise/
// Also https://forum.unity.com/threads/2d-3d-4d-optimised-perlin-noise-cg-hlsl-library-cginc.218372/
// Also https://gist.github.com/fadookie/25adf86ae7e2753d717c

/* Permutation in mod 289. */
#define NOISE_SIMPLEX_1_DIV_289 0.00346020761245674740484429065744

highp float mod289(highp float x) {
    return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0;
}

highp vec2 mod289(highp vec2 x) {
    return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0;
}

highp vec3 mod289(highp vec3 x) {
    return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0;
}

highp vec4 mod289(highp vec4 x) {
    return x - floor(x * NOISE_SIMPLEX_1_DIV_289) * 289.0;
}

highp float permute289(highp float x) {
    return mod289((x * 34.0 + 1.0) * x);
}

highp vec3 permute289(highp vec3 x) {
    return mod289((x * 34.0 + 1.0) * x);
}

highp vec4 permute289(highp vec4 x) {
    return mod289((x * 34.0 + 1.0) * x);
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

/* 2D simplex noise [-1, 1], based on https://github.com/stegu/webgl-noise/
 */
highp float simplexNoise(highp vec2 v) {
    const highp vec4 C = vec4(
        0.211324865405187,   // (3.0-sqrt(3.0))/6.0
        0.366025403784439,   // 0.5*(sqrt(3.0)-1.0)
        -0.577350269189626,  // -1.0 + 2.0 * C.x
        0.024390243902439);  // 1.0 / 41.0

    // First corner
    highp vec2 i  = floor(v + dot(v, C.yy));
    highp vec2 x0 = v -   i + dot(i, C.xx);

    // Other corners
    highp vec2 i1  = x0.x > x0.y ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    highp vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Permutations
    i = mod289(i); // Avoid truncation effects in permutation
    highp vec3 p =
        permute289(
            permute289(
                i.y + vec3(0.0, i1.y, 1.0)
                ) + i.x + vec3(0.0, i1.x, 1.0)
            );

    highp vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
    m = m*m;
    m = m*m;

    // Gradients: 41 points uniformly over a line, mapped onto a
    // diamond.  The ring size 17*17 = 289 is close to a multiple of
    // 41 (41*7 = 287)
    highp vec3 x  = 2.0 * fract(p * C.www) - 1.0;
    highp vec3 h  = abs(x) - 0.5;
    highp vec3 ox = round(x);
    highp vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    m *= inversesqrt(a0 * a0 + h * h);

    // Compute final noise value at P
    highp vec3 g;
    g.x  = a0.x  * x0.x   + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

/* 3D simplex noise [-1, 1], based on https://github.com/stegu/webgl-noise/
 */
highp float simplexNoise(highp vec3 v) {
    const highp vec2 C = vec2(
        0.166666666666666667,  // 1/6
        0.333333333333333333); // 1/3
    const highp vec4 D = vec4(0.0, 0.5, 1.0, 2.0);

    // First corner
    highp vec3 i  = floor(v + dot(v, C.yyy));
    highp vec3 x0 = v -   i + dot(i, C.xxx);

    // Other corners
    highp vec3 g  = step(x0.yzx, x0.xyz);
    highp vec3 l  = 1.0 - g;
    highp vec3 i1 = min(g.xyz, l.zxy);
    highp vec3 i2 = max(g.xyz, l.zxy);

    //         x0 = x0 - 0.0 + 0.0 * C.xxx;
    //         x1 = x0 - i1  + 1.0 * C.xxx;
    //         x2 = x0 - i2  + 2.0 * C.xxx;
    //         x3 = x0 - 1.0 + 3.0 * C.xxx;
    highp vec3 x1 = x0 - i1 + C.xxx;
    highp vec3 x2 = x0 - i2 + C.yyy; // 2.0*C.x = 1/3 = C.y
    highp vec3 x3 = x0 - D.yyy; // -1.0+3.0*C.x = -0.5 = -D.y

    // Permutations
    i = mod289(i);
    highp vec4 p =
        permute289(
            permute289(
                permute289(
                    i.z + vec4(0.0, i1.z, i2.z, 1.0)
                    ) + i.y + vec4(0.0, i1.y, i2.y, 1.0)
                ) + i.x + vec4(0.0, i1.x, i2.x, 1.0)
            );

    // Gradients: 7x7 points over a square, mapped onto an octahedron.
    // The ring size 17*17 = 289 is close to a multiple of 49 (49*6 =
    // 294)
    const highp float n_ = 0.142857142857; // 1.0/7.0
    highp vec3  ns = n_ * D.wyz - D.xzx;

    highp vec4 j = p - 49.0 * floor(p * ns.z * ns.z);  //  mod(p, 7*7)

    highp vec4 x_ = floor(j * ns.z);
    highp vec4 y_ = floor(j - 7.0 * x_);    // mod(j, N)

    highp vec4 x = x_ * ns.x + ns.yyyy;
    highp vec4 y = y_ * ns.x + ns.yyyy;
    highp vec4 h = 1.0 - abs(x) - abs(y);

    highp vec4 b0 = vec4(x.xy, y.xy);
    highp vec4 b1 = vec4(x.zw, y.zw);

    highp vec4 s0 = floor(b0) * 2.0 + 1.0;
    highp vec4 s1 = floor(b1) * 2.0 + 1.0;
    highp vec4 sh = -step(h, vec4(0.0));

    highp vec4 a0 = b0.xzyw + s0.xzyw * sh.xxyy;
    highp vec4 a1 = b1.xzyw + s1.xzyw * sh.zzww;

    highp vec3 p0 = vec3(a0.xy, h.x);
    highp vec3 p1 = vec3(a0.zw, h.y);
    highp vec3 p2 = vec3(a1.xy, h.z);
    highp vec3 p3 = vec3(a1.zw, h.w);

    // Normalise gradients
    highp vec4 norm = inversesqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;

    // Mix final noise value
    highp vec4 m = max(0.5 - vec4(dot(x0, x0), dot(x1, x1), dot(x2, x2), dot(x3, x3)), 0.0);
    m = m * m;
    return 42.0 *
        dot(m * m, vec4(dot(p0, x0), dot(p1, x1), dot(p2, x2), dot(p3, x3)));
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
    i = mod289(i);
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

    highp vec4 p0 = grad4(j0  , ip);
    highp vec4 p1 = grad4(j1.x, ip);
    highp vec4 p2 = grad4(j1.y, ip);
    highp vec4 p3 = grad4(j1.z, ip);
    highp vec4 p4 = grad4(j1.w, ip);

    // Normalise gradients
    highp vec4 norm = inversesqrt(vec4(dot(p0, p0), dot(p1, p1), dot(p2, p2), dot(p3, p3)));
    p0 *= norm.x;
    p1 *= norm.y;
    p2 *= norm.z;
    p3 *= norm.w;
    p4 *= inversesqrt(dot(p4, p4));

    // Mix contributions from the five corners
    highp vec3 m0 = max(0.5 - vec3(dot(x0, x0), dot(x1, x1), dot(x2, x2)), 0.0);
    highp vec2 m1 = max(0.5 - vec2(dot(x3, x3), dot(x4, x4)             ), 0.0);
    m0 = m0 * m0;
    m1 = m1 * m1;
    return 49.0 *
        ( dot(m0 * m0, vec3(dot(p0, x0), dot(p1, x1), dot(p2, x2))) +
          dot(m1 * m1, vec2(dot(p3, x3), dot(p4, x4))) );
}

/* Generate a 2D fBM noise [0, 1]. See
 * https://thebookofshaders.com/13/
 */
highp float fBM(const int octaves, const float lowerBound, const float upperBound, highp vec2 st) {
    // Initial values
    highp float value = 0.0;
    highp float amplitude = 0.5;

    // Loop of octaves
    for (int i = 0; i < octaves; i++) {
        value += amplitude * (simplexNoise(st) * 0.5 + 0.5);

        if (value >= upperBound) {
            /* Optimization (#29): We have already reached the upper
             * bound so no further accumulations can affect the final
             * result.
             */
            break;
        }
        else if (value + amplitude <= lowerBound) {
            /* Optimization (#29): The maximum of accumulated noise
             * converges to value + amplitude at this point (at i →
             * ∞), which isn't going to reach the cutoff
             * threshold. */
            break;
        }

        st        *= 2.0;
        amplitude *= 0.5;
    }

    return smoothstep(lowerBound, upperBound, value);
}

#endif /* NATURAL_MYSTIC_NOISE_H_INCLUDED */
