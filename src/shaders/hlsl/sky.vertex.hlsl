#include "ShaderConstants.fxh"
#include "natural-mystic-color.fxh"

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
    float4 skyColor : skyColor;
    float4 cloudColor : cloudColor;
    float3 worldPos : worldPos;
    float camDist : camDist;
#ifdef GEOMETRY_INSTANCEDSTEREO
	uint instanceID : SV_InstanceID;
#endif
#ifdef VERTEXSHADER_INSTANCEDSTEREO
	uint renTarget_id : SV_RenderTargetArrayIndex;
#endif
};

ROOT_SIGNATURE
void main(in VS_Input VSInput, out PS_Input PSInput)
{
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
    PSInput.skyColor = lerp( CURRENT_COLOR, FOG_COLOR, VSInput.color.r );
    PSInput.worldPos = VSInput.position.xyz;
    PSInput.camDist = length(PSInput.worldPos);

    /* The color of clouds resembles that of the sky, but is more
     * close to that of the fog. This way we will have something like
     * a sunlight-reflected-by-clouds effect (but more
     * lightgray-ish). */
    float brightness = clamp(desaturate(FOG_COLOR.rgb, 1.0).r + 0.05, 0.105, 1.0);
    PSInput.cloudColor     = lerp(PSInput.skyColor, FOG_COLOR, 0.9);
    PSInput.cloudColor     = lerp(PSInput.cloudColor, 1.0, brightness);
    PSInput.cloudColor.rgb = desaturate(PSInput.cloudColor.rgb, 0.4);
}

// Local Variables:
// mode: hlsl
// indent-tabs-mode: nil
// End: