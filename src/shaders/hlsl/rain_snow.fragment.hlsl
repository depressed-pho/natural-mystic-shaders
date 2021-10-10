#include "ShaderConstants.fxh"
#include "natural-mystic-color.fxh"

struct PS_Input {
	float4 position : SV_Position;
	float2 uv : TEXCOORD_0;
	float4 color : COLOR;
	float4 worldPosition : TEXCOORD_1;
	float4 fogColor : FOG_COLOR;
};

struct PS_Output {
	float4 color : SV_Target;
};

ROOT_SIGNATURE
void main(in PS_Input PSInput, out PS_Output PSOutput)
{	
	float4 albedo = TEXTURE_0.Sample( TextureSampler0, PSInput.uv);

#ifdef ALPHA_TEST
	if (albedo.a < 0.5)
		discard;
#endif

	albedo.a *= PSInput.color.a;

	float2 occlusionUV = PSInput.worldPosition.xz;
	float4 occlusionTexture = TEXTURE_0.Sample( TextureSampler1, occlusionUV);

#ifndef FLIP_OCCLUSION
#define OCCLUSION_OPERATOR <
#else
#define OCCLUSION_OPERATOR >
#endif

#ifdef SNOW
#define OCCLUSION_HEIGHT occlusionTexture.g
#define OCCLUSION_LUMINANCE occlusionTexture.r
#else
#define OCCLUSION_HEIGHT occlusionTexture.a
#define OCCLUSION_LUMINANCE occlusionTexture.b
#endif

	// clamp the uvs
#ifndef NO_OCCLUSION
	if ( occlusionUV.x >= 0.0 && occlusionUV.x <= 1.0 && 
		 occlusionUV.y >= 0.0 && occlusionUV.y <= 1.0 && 
		 PSInput.worldPosition.y OCCLUSION_OPERATOR OCCLUSION_HEIGHT) {
		albedo.a = 0.0;
	}
#endif

	float mixAmount = (PSInput.worldPosition.y - OCCLUSION_HEIGHT)*25.0;
	float2 lightingUVs = float2(OCCLUSION_LUMINANCE, 1.0);
	lightingUVs.x = lerp(lightingUVs.x, 0.0, mixAmount);

	float3 lighting = TEXTURE_2.Sample( TextureSampler2, occlusionUV ).rgb;
	float4 finalOutput = albedo;
	finalOutput.rgb *= lighting.rgb;

	/* The color of particles should be highly desaturated because it
	 * looks ugly when the rain looks blue. */
	finalOutput.rgb = desaturate(finalOutput.rgb, 0.9);

	//apply fog
	PSOutput.color.rgb = lerp( finalOutput.rgb, PSInput.fogColor.rgb, PSInput.fogColor.a );
	PSOutput.color.a = finalOutput.a;
}

// Local Variables:
// mode: hlsl
// indent-tabs-mode: t
// End:
