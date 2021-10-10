#include "ShaderConstants.fxh"
#include "natural-mystic-cloud.fxh"
#include "natural-mystic-config.fxh"

struct PS_Input
{
    float4 position : SV_Position;
    float4 skyColor : skyColor;
    float4 cloudColor : cloudColor;
    float3 worldPos : worldPos;
    float camDist : camDist;
};

struct PS_Output
{
    float4 color : SV_Target;
};

ROOT_SIGNATURE
void main(in PS_Input PSInput, out PS_Output PSOutput) {
#if defined(ENABLE_FBM_CLOUDS)

    /* NOTE: It seems modifying materials/fancy.json takes no effect
     * on 1.8. We want to reduce the number of octaves when
     * !defined(FANCY) but we can't do it for now, because FANCY gets
     * never defined in this shader. */
    static const int octaves = 6;

    /* We are going to perform a (sort of) volumetric ray marching to
     * compute self-casting shadows of clouds (#46), but with only a
     * few steps. This is because ray marching is terribly expensive
     * as we cannot precompute noises in a texture and instead we have
     * to generate them on the fly. See also
     * http://www.iquilezles.org/www/articles/dynclouds/dynclouds.htm */
    float density = cloudMap(octaves, 0.5, 0.85, TOTAL_REAL_WORLD_TIME, PSInput.worldPos);
    float4 shadedCloud = lerp(float4(PSInput.cloudColor.rgb, 0.0), PSInput.cloudColor, density);

#  if defined(ENABLE_CLOUD_SHADE)
    /* Optimization: Don't bother to do it when there are no clouds at
     * the current position. */
    if (density > 0.0) {
        /* The game doesn't tell us where the sun or the moon is,
         * which is so unfortunate. We have to assume they are always
         * at a fixed point. */
        static const float3 sunMoonPos = float3(-0.3, 4.0, 0);

        static const int   numSteps = 1; /* Yes, it has to be this few, or
                                   * we'll get a lag. */
        static const float stepSize = 0.2;
        float3        rayStep  = normalize(sunMoonPos - PSInput.worldPos) * stepSize;
        float3  rayPos   = PSInput.worldPos;
        float       inside   = 0.0;
        for (int i = 0; i < numSteps; i++) {
            rayPos += rayStep;
            float height = cloudMap(octaves / 2, 0.4, 1.0, TOTAL_REAL_WORLD_TIME, rayPos);
            inside += max(0.0, height - (rayPos.y - PSInput.worldPos.y));
        }
        /* Average of height differences. This isn't a distance of ray
         * traveled inside clouds in a normal sense, but if we do it
         * strictly we get severe banding artifacts (because of the
         * number of steps being too few). */
        inside /= float(numSteps);

        float brightness = PSInput.cloudColor.r;
        shadedCloud.rgb = lerp(
            shadedCloud.rgb + 0.1 * brightness, // highlight
            max(0.0, shadedCloud.rgb - 0.2 * brightness), // shade
            inside);
    }
#  endif /* defined(ENABLE_CLOUD_SHADE) */
    shadedCloud.rgb = lerp(PSInput.skyColor.rgb, shadedCloud.rgb, shadedCloud.a);

    /* Clouds near the horizon should be blended back to the sky
     * color, or otherwise the planar nature of the sky plane will be
     * even more apparent. */
    PSOutput.color = lerp(shadedCloud, PSInput.skyColor, smoothstep(0.9, 1.0, PSInput.camDist));

#else
    PSOutput.color = PSInput.skyColor;
#endif /* ENABLE_FBM_CLOUDS */
}

// Local Variables:
// mode: hlsl
// indent-tabs-mode: nil
// End: