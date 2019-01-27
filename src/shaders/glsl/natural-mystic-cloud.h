// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_CLOUD_H_INCLUDED)
#define NATURAL_MYSTIC_CLOUD_H_INCLUDED 1

#include "natural-mystic-noise.h"

/* Generate a pattern of clouds based on a world position. */
highp float cloudMap(int octaves, float lowerBound, float upperBound, highp float time, highp vec3 pos) {
    /* Use of highp is essential here, as the uniform TIME in mediump
     * starts to lose precision within 10 minutes.
     */
    const highp vec2 resolution = vec2(1.4, 1.4);

    highp vec2 st = pos.xz / resolution;
    /* The inverse of the speed (512) should be a power of two in
     * order to avoid a precision loss.
     */
    st.y += time / 512.0;

    /* We intentionally throw away some
     * of the precision so we get somewhat sparse noise.
     */
    return fBM(octaves, lowerBound, upperBound, st * 3.0);
}

#endif /* !defined(NATURAL_MYSTIC_CLOUD_H_INCLUDED) */
