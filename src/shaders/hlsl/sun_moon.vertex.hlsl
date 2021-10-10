#include "ShaderConstants.fxh"
#include "natural-mystic-config.fxh"
#include "natural-mystic-hacks.fxh"

struct VS_Input
{
    float3 position : POSITION;
    float2 uv : TEXCOORD_0;
#ifdef INSTANCEDSTEREO
	uint instanceID : SV_InstanceID;
#endif
};


struct PS_Input
{
    float4 position : SV_Position;
    float2 uv : TEXCOORD_0;

#if defined(ENABLE_SHADER_SUN_MOON)

/* This is the vertex position is the model-local space whose origin
 * is the center of the sun or the moon.
 */
float2 localPos : localPos;

/* 1.0 when it's dusk or dawn. */
float duskOrDown : duskOrDown;

/* 1.0 when it's night. */
float night : night;

/* The phase of the moon. */
float phase : phase;

#endif /* defined(ENABLE_SHADER_SUN_MOON) */

#ifdef GEOMETRY_INSTANCEDSTEREO
	uint instanceID : SV_InstanceID;
#endif
#ifdef VERTEXSHADER_INSTANCEDSTEREO
	uint renTarget_id : SV_RenderTargetArrayIndex;
#endif
};

/* Translate the texture UV into the phase of moon. This function
 * returns [0, 2π) where 0 is the new moon and π is the full
 * moon. This is a hack.
 */
float moonPhase(float2 texUV) {
    float x = floor(texUV.x * 4.0) * 0.25; // {0, 0.25, 0.5, 0.75}
    float y = step(0.5, texUV.y);          // {0, 1}
    return (x + y) * 3.14159;
}

ROOT_SIGNATURE
void main(in VS_Input VSInput, out PS_Input PSInput)
{

    PSInput.uv = VSInput.uv;

#if defined(ENABLE_SHADER_SUN_MOON)

    /* Enlarge the sun and the moon as the default size is too small
     * for us. We are going to draw actual shape of the sun and the
     * moon inside these vertices.
     */
    float4 p = float4(VSInput.position, 1) * float2(10.0, 1.0).xyxy;
#ifdef INSTANCEDSTEREO
	int i = VSInput.instanceID;
	PSInput.position = mul( WORLDVIEWPROJ_STEREO[i], p );
#else
	PSInput.position = mul(WORLDVIEWPROJ, p);
#endif

    /* Apply some rotation to the moon. */
    PSInput.localPos = mul(float2x2(0.8, 0.6, -0.6, 0.8), p.xz);

    /* Hacky time detections. */
    PSInput.duskOrDown = isDuskOrDawn(FOG_COLOR);
    PSInput.night      = isNight(FOG_COLOR);
    PSInput.phase      = moonPhase(VSInput.uv);

#else
    // Copied from the vanilla uv.vertex
#ifdef INSTANCEDSTEREO
	int i = VSInput.instanceID;
	PSInput.position = mul( WORLDVIEWPROJ_STEREO[i], float4( VSInput.position, 1 ) );
#else
	PSInput.position = mul(WORLDVIEWPROJ, float4(VSInput.position, 1));
#endif
#endif /* defined(ENABLE_SHADER_SUN_MOON) */

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