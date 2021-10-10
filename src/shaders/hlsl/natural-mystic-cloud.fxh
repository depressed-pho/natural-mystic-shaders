// -*- hlsl -*-
#if !defined(NATURAL_MYSTIC_CLOUD_FXH_INCLUDED)
#define NATURAL_MYSTIC_CLOUD_FXH_INCLUDED 1

#include "natural-mystic-noise.fxh"

/* Generate a pattern of clouds based on a world position. */
float cloudMap(int octaves, float lowerBound, float upperBound, float time, float3 pos) {
    static const float2 resolution = float2(1.4, 1.4);

    float2 st = pos.xz / resolution;
    /* The inverse of the speed (512) should be a power of two in
     * order to avoid a precision loss.
     */
    st.y += time / 512.0;

    /* We intentionally throw away some
     * of the precision so we get somewhat sparse noise.
     */
    return fBM(octaves, lowerBound, upperBound, st * 3.0);
}

#endif /* !defined(NATURAL_MYSTIC_CLOUD_FXH_INCLUDED) */
