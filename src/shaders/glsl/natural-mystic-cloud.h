// -*- glsl -*-
#if !defined(NATURAL_MYSTIC_CLOUD_H_INCLUDED)
#define NATURAL_MYSTIC_CLOUD_H_INCLUDED 1

#include "natural-mystic-noise.h"

/* Generate a pattern of clouds based on a world position. */
highp float cloudMap(int octaves, highp float time, highp vec3 pos) {
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
    highp float result = fBM(octaves, st * 3.0);
    result = smoothstep(0.5, 1.0, result) * 1.5;
    result = clamp(result, 0.0, 1.0);

    /* Workaround for MCPE-39749: the uniform TIME might not be a
     * number. See https://bugs.mojang.com/browse/MCPE-39749
     */
    result = isnan(result) || isinf(result) ? 0.0 : result;
    return result;
}

#endif /* !defined(NATURAL_MYSTIC_CLOUD_H_INCLUDED) */
