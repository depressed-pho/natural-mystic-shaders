#include "ShaderConstants.fxh"
#include "natural-mystic-config.fxh"
#include "natural-mystic-noise.fxh"

struct VS_Input
{
    float3 position : POSITION;
    float4 color : COLOR;
#ifdef INSTANCEDSTEREO
	uint instanceID : SV_InstanceID;
#endif
};


struct PS_Input
{
    float4 position : SV_Position;
    float4 color : COLOR;
#ifdef GEOMETRY_INSTANCEDSTEREO
	uint instanceID : SV_InstanceID;
#endif
#ifdef VERTEXSHADER_INSTANCEDSTEREO
	uint renTarget_id : SV_RenderTargetArrayIndex;
#endif
};

/* Generate a pattern of brightness of stars based on a world
 * position. */
float stars(float3 pos) {
    static const float2 resolution = 0.3;

    float2 st = pos.xz / resolution;
    return (simplexNoise(st) + 1.0) * 0.5;
}

ROOT_SIGNATURE
void main(in VS_Input VSInput, out PS_Input PSInput)
{
    PSInput.color = VSInput.color;

#if defined(ENABLE_RANDOM_STARS)
    PSInput.color.rgb *= stars(VSInput.position.xyz);
#endif

#ifdef INSTANCEDSTEREO
	int i = VSInput.instanceID;
	PSInput.position = mul( WORLDVIEWPROJ_STEREO[i], float4( VSInput.position, 1 ) );
#else
	PSInput.position = mul(WORLDVIEWPROJ, float4(VSInput.position, 1));
#endif
#ifdef GEOMETRY_INSTANCEDSTEREO
	PSInput.instanceID = VSInput.instanceID;
#endif 
#ifdef VERTEXSHADER_INSTANCEDSTEREO
	PSInput.renTarget_id = VSInput.instanceID;
#endif
}

// Local Variables:
// mode: hlsl
// indent-tabs-mode: nil
// End: