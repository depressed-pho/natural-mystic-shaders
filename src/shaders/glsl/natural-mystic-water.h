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

highp vec3 geometricWaterWave(highp vec3 wPos, highp float time, out highp vec3 normal) {
    normal = vec3(0.0, 1.0, 0.0);
    return wPos;
}

#endif /* !defined(NATURAL_MYSTIC_WATER_H_INCLUDED) */
