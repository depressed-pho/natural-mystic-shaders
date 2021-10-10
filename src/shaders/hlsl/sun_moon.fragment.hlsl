#include "ShaderConstants.fxh"
#include "util.fxh"
#include "natural-mystic-config.fxh"
#include "natural-mystic-hacks.fxh"
#include "natural-mystic-noise.fxh"

struct PS_Input
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD_0_FB_MSAA;
    float2 localPos : localPos;
    float duskOrDown : duskOrDown;
    float night : night;
    float phase : phase;
};

struct PS_Output
{
    float4 color : SV_Target;
};

float4 renderSun(float2 pos, float duskOrDown) {
    const float radius     = 0.08;
    const float sharpness  = 0.2;  // The smaller the sharper the edge will be.
    const float colorScale = 2.5;
    const float3  baseColor  = float3(1.0, 0.55, 0.15);

    /* The fog color is white during the daytime and is red at dusk
     * and dawn. We use it as the color of the sun. We also hide the
     * sun when it's raining, hence the CURRENT_COLOR.a.
     */
    float4 sunColor = float4(FOG_COLOR.rgb * baseColor * colorScale, CURRENT_COLOR.a);

    /* The body of the sun in a Signed Distance Field. */
    float sdf        = length(pos) - radius;
    float brightness = 1.0 - smoothstep(0.0, radius * sharpness, sdf);

    /* Render halo, or sort of. When it's daytime and is clear, quite
     * a large area around the sun should be rendered extremely
     * brightly. We do this by using the distance between the current
     * pixel and the border of the sun as the factor of the
     * brightness, i.e. the closer the brighter. */
    float halo = (1.0 - duskOrDown) * 0.7;
    brightness = max(brightness, (1.0 - sdf) * halo);

    return clamp(sunColor * brightness, 0.0, 1.0);
}

/* Based on https://www.shadertoy.com/view/XsdGzX
 * and http://learnwebgl.brown37.net/09_lights/lights_diffuse.html
 */
float diffuseSphere(float2 pos, float radius, float3 light) {
    float sq = radius*radius - pos.x*pos.x - pos.y*pos.y;

    if (sq < 0.0) {
        return 0.0;
    }
    else {
        float z      = sqrt(sq);
        float3  normal = normalize(float3(pos, z));
        return max(0.0, dot(normal, light));
    }
}

/* Huge thanks for ESBE-2G shaders. I took its moon renderer as a
 * reference (although I didn't use it directly).
 */
float4 renderMoon(float2 pos, float phase) {
    const float radius      = 0.11;
    const float sharpness   = 0.15; // The smaller the sharper the edge will be.
    const float3  baseColor = float3(1.0, 0.95, 0.81);
    const float2  resolution = 0.06;

    /* We hide the moon when it's raining, hence the
     * CURRENT_COLOR.a.
     */
    float4 moonColor = float4(baseColor, CURRENT_COLOR.a);

    /* The body of the moon in a Signed Distance Field. */
    float pDistance  = length(pos);
    float sdf        = pDistance - radius;

    /* Compute the light floattor based on the phase of moon. */
    float3 light = float3(sin(phase), 0.0, -cos(phase));

    /* Perform a diffuse lighting to render the phase of moon. */
    float diffuse = diffuseSphere(pos, radius, light);
    /* Hacky adjustment of the diffuse light. */
    float brightness = smoothstep(0.2, 0.8, min(diffuse + 0.3, 1.0));

    /* Generate the texture of the moon with 2D simplex noise. */
    float tex = simplexNoise(pos / resolution);
    brightness *= 1.0 - clamp(tex, 0.0, 1.0) * 0.05;

    /* Apply the sharpness to the surface. */
    brightness *= 1.0 - smoothstep(radius - radius * sharpness, radius, pDistance);

    return clamp(moonColor * brightness, 0.0, 1.0);
}

ROOT_SIGNATURE
void main(in PS_Input PSInput, out PS_Output PSOutput) {
#if defined(ENABLE_SHADER_SUN_MOON)

    if (PSInput.night < 0.5) {
        PSOutput.color = renderSun(PSInput.localPos, PSInput.duskOrDown);
    }
    else {
        PSOutput.color = renderMoon(PSInput.localPos, PSInput.phase);
    }

#else /* defined(ENABLE_SHADER_SUN_MOON) */
    // Copied from the vanilla texture_ccolor.fragment

#if !defined(TEXEL_AA) || !defined(TEXEL_AA_FEATURE) || (VERSION < 0xa000 /*D3D_FEATURE_LEVEL_10_0*/) 
	float4 diffuse = TEXTURE_0.Sample(TextureSampler0, PSInput.uv);
#else
	float4 diffuse = texture2D_AA(TEXTURE_0, TextureSampler0, PSInput.uv);
#endif

#ifdef ALPHA_TEST
    if( diffuse.a < 0.5 )
    {
        discard;
    }
#endif

#ifdef IGNORE_CURRENTCOLOR
    PSOutput.color = diffuse;
#else
    PSOutput.color = CURRENT_COLOR * diffuse;
#endif

#ifdef WINDOWSMR_MAGICALPHA
    // Set the magic MR value alpha value so that this content pops over layers
    PSOutput.color.a = 133.0f / 255.0f;
#endif
#endif /* !defined(ENABLE_SHADER_SUN_MOON) */
}

// Local Variables:
// mode: hlsl
// indent-tabs-mode: nil
// End: