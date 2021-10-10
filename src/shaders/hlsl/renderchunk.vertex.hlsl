#include "ShaderConstants.fxh"
#include "natural-mystic-color.fxh"
#include "natural-mystic-config.fxh"
#include "natural-mystic-fog.fxh"
#include "natural-mystic-hacks.fxh"
#include "natural-mystic-light.fxh"
#include "natural-mystic-water.fxh"

struct VS_Input {
	float3 position : POSITION;
	float4 color : COLOR;
	float2 uv0 : TEXCOORD_0;
	float2 uv1 : TEXCOORD_1;
#ifdef INSTANCEDSTEREO
	uint instanceID : SV_InstanceID;
#endif
};


struct PS_Input {
	float4 position : SV_Position;

#ifndef BYPASS_PIXEL_SHADER
	lpfloat4 color : COLOR;
	snorm float2 uv0 : TEXCOORD_0_FB_MSAA;
	snorm float2 uv1 : TEXCOORD_1_FB_MSAA;
#endif

/* Workaround for https://bugs.mojang.com/browse/MCPE-40059 */
#if defined(MCPE40059)
float3 wPos : wPos;
float cameraDist : camDist; // Vertex normal in the world space. Only
float3 vNormal : vNormal;   // defined when waterPlane > 0.0.

float flickerFactor : flickerFactor;
float desatFactor : desatFactor;
float clearWeather : clearWeather; // [0, 1]
float waterFlag : waterFlag;       // 0.0 or 1.0
float waterPlane : waterPlane;     // [0, 1]
#endif

#ifdef FOG
	float4 fogColor : FOG_COLOR;
#endif
#ifdef GEOMETRY_INSTANCEDSTEREO
	uint instanceID : SV_InstanceID;
#endif
#ifdef VERTEXSHADER_INSTANCEDSTEREO
	uint renTarget_id : SV_RenderTargetArrayIndex;
#endif
};

/* Notes on different kinds of positions:
 *
 * - attribute float4 VSInput.position: Relative position to the current
 *   chunk origin in [0.0, 16.0]. 
 *
 * - float4 CHUNK_ORIGIN_AND_SCALE: Its ".xyz" components
 *   contain the origin of the current chunk in the world space
 *   relative to the camera (see below). Its ".w" component is for
 *   scaling chunk-relative positions into this space.
 *
 * - float3 worldPos: Vertex position in the world space relative
 *   to the camera in (-∞, ∞). Its ".w" component is always 1.0. This
 *   isn't the same as the view space as the camera angle doesn't
 *   affect it (see
 *   http://www.codinglabs.net/article_world_view_projection_matrix.aspx).
 *
 * - float3 PSInput.wPos: The same as worldPos.xyz.
 *
 * - float3 VIEW_POS: Unknown, apparently all zero.
 */

static const float rA = 1.0;
static const float rB = 1.0;
static const float3 UNIT_Y = float3(0, 1, 0);
static const float DIST_DESATURATION = 56.0 / 255.0; //WARNING this value is also hardcoded in the water color, don'tchange


ROOT_SIGNATURE
void main(in VS_Input VSInput, out PS_Input PSInput)
{
#ifndef BYPASS_PIXEL_SHADER
	PSInput.uv0 = VSInput.uv0;
	PSInput.uv1 = VSInput.uv1;
	PSInput.color = VSInput.color;
#endif

#ifdef AS_ENTITY_RENDERER
	#ifdef INSTANCEDSTEREO
		int i = VSInput.instanceID;
		PSInput.position = mul(WORLDVIEWPROJ_STEREO[i], float4(VSInput.position, 1));
	#else
		PSInput.position = mul(WORLDVIEWPROJ, float4(VSInput.position, 1));
	#endif
		float3 worldPos = PSInput.position;
#else
		float3 worldPos = (VSInput.position.xyz * CHUNK_ORIGIN_AND_SCALE.w) + CHUNK_ORIGIN_AND_SCALE.xyz;
	
		// Transform to view space before projection instead of all at once to avoid floating point errors
		// Not required for entities because they are already offset by camera translation before rendering
		// World position here is calculated above and can get huge
	#ifdef INSTANCEDSTEREO
		int i = VSInput.instanceID;
	
		PSInput.position = mul(WORLDVIEW_STEREO[i], float4(worldPos, 1 ));
		PSInput.position = mul(PROJ_STEREO[i], PSInput.position);
	
	#else
		PSInput.position = mul(WORLDVIEW, float4( worldPos, 1 ));
		PSInput.position = mul(PROJ, PSInput.position);
	#endif

#endif
#ifdef GEOMETRY_INSTANCEDSTEREO
		PSInput.instanceID = VSInput.instanceID;
#endif 
#ifdef VERTEXSHADER_INSTANCEDSTEREO
		PSInput.renTarget_id = VSInput.instanceID;
#endif

#if defined(MCPE40059)
	/* THINKME: Ideally this should be the absolute position of the
	 * vertex in the world space without getting affected by the
	 * camera position, but there is apparently no way to reconstruct
	 * the space from what we get from the game code (#36).
	 */
	PSInput.wPos = worldPos.xyz;

	PSInput.vNormal = 0.0;
	PSInput.flickerFactor = 1.0;
#  if defined(ENABLE_TORCH_FLICKER)
	if (PSInput.uv1.x > 0.0) {
		PSInput.flickerFactor = torchLightFlicker(worldPos.xyz, TOTAL_REAL_WORLD_TIME);
	}
#  endif
#endif /* defined(MCPE40059) */

///// find distance from the camera
	float3 relPos = -worldPos.xyz;
	float cameraDepth = length(relPos);
	PSInput.cameraDist = cameraDepth / RENDER_DISTANCE;

	/* Reduce the contrast of far objects (#5). The overall color
	 * should lean towards the ambient. Note that cameraDist is a
	 * normalized camera distance being 1.0 at the point where the far
	 * terrain fog ends. */
#if defined(FANCY) && defined(MCPE40059)
	PSInput.desatFactor = exponentialFog(float2(0.0, 4.0), PSInput.cameraDist);
#endif

	/* Detect the weather on the Overworld. */
#if defined(MCPE40059)
#  if defined(FOG)
	PSInput.clearWeather = isClearWeather(FOG_CONTROL);
#  else
	PSInput.clearWeather = 1.0;
#  endif
#endif

///// apply fog

#ifdef FOG
	float len = PSInput.cameraDist;
	#ifdef ALLOW_FADE
		len += RENDER_CHUNK_FOG_ALPHA;
	#endif

    PSInput.fogColor.rgb = FOG_COLOR.rgb;
#  if defined(FOG_TYPE)
#    if FOG_TYPE == FOG_TYPE_LINEAR
	PSInput.fogColor.a = linearFog(FOG_CONTROL, len);
#    elif FOG_TYPE == FOG_TYPE_EXP
	PSInput.fogColor.a = exponentialFog(FOG_CONTROL, len);
#    elif FOG_TYPE == FOG_TYPE_EXP2
	PSInput.fogColor.a = exponentialSquaredFog(FOG_CONTROL, len);
#    endif
#  else
	PSInput.fogColor.a = 0.0; /* Fog disabled? Really?? */
#  endif /* defined(FOG_TYPE) */
#endif /* FOG */

	/* Waves */
	float3 hsvColor = rgb2hsv(VSInput.color.rgb);
#if defined(MCPE40059)
#  if defined(ENABLE_FANCY_WATER)
	PSInput.waterFlag  = isWater(hsvColor) ? 1.0 : 0.0;
#  else
	PSInput.waterFlag  = 0.0;
#  endif
	PSInput.waterPlane = 0.0;
#endif
#if !defined(BYPASS_PIXEL_SHADER) && !defined(AS_ENTITY_RENDERER) && defined(ENABLE_WAVES) && defined(MCPE40059)
#  if defined(ALPHA_TEST)
	/* ALPHA_TEST means that the block being rendered isn't a solid
	 * opaque one. This excludes grass blocks especially. */
	bool grassFlag = isGrass(hsvColor);
#  else
	static const bool grassFlag = false;
#  endif
	if (grassFlag) {
		float3 posw = abs(VSInput.position.xyz - 8.0);
		float wave = sin(TOTAL_REAL_WORLD_TIME * 3.5 + 2.0 * posw.x + 2.0 * posw.z + posw.y);
		float amplitude = 0.015;
		/* Reduce the amplitude if it's indoor, i.e. the sunlight
		 * level is low (#85). */
		PSInput.position.x += wave * amplitude * smoothstep(0.7, 1.0, PSInput.uv1.y);
	}
	else if (PSInput.waterFlag > 0.5) {
		/* We want water to swell in proportion to its volume. The more
		 * the vertex is close to the ground (i.e. integral points in the
		 * world position), the less the vertex should swell. Without this
		 * tweak the water will leave the ground. */
		float volume = frac(VSInput.position.y);
		if (volume > 0.0) {
			float3 wPos1     = waterWaveGeometric(PSInput.wPos, TOTAL_REAL_WORLD_TIME, PSInput.vNormal);
			float3 wPosDelta = (wPos1 - PSInput.wPos) * volume;
			/* Also reduce the amplitude depending on the sunlight
			 * level. We do the same for wave normal in the fragment
			 * shader. */
			worldPos.xyz += wPosDelta * smoothstep(0.5, 1.0, PSInput.uv1.y);
			PSInput.wPos     = worldPos.xyz;
			#ifdef INSTANCEDSTEREO
				int i = VSInput.instanceID;
			
				PSInput.position = mul(WORLDVIEW_STEREO[i], float4(worldPos, 1 ));
				PSInput.position = mul(PROJ_STEREO[i], PSInput.position);
			
			#else
				PSInput.position = mul(WORLDVIEW, float4( worldPos, 1 ));
				PSInput.position = mul(PROJ, PSInput.position);
			#endif
		}

#  if defined(FANCY)
		/* When we know the surface normal we can do something
		 * advanced. */
		if (isWaterPlane(float4(VSInput.position, 1.0))) {
			PSInput.waterPlane = 1.0;
		}
		// The default opacity of water is way too high. Reduce it.
		PSInput.color.a *= 0.5;
#  endif /* defined(FANCY) */
	}
#endif /* !defined(BYPASS_PIXEL_SHADER) && !defined(AS_ENTITY_RENDERER) && defined(ENABLE_WAVES) */

///// blended layer (mostly water) magic
#ifdef BLEND
	//Mega hack: only things that become opaque are allowed to have vertex-driven transparency in the Blended layer...
	//to fix this we'd need to find more space for a flag in the vertex format. color.a is the only unused part
	bool shouldBecomeOpaqueInTheDistance = PSInput.color.a < 0.95;
	if(shouldBecomeOpaqueInTheDistance) {
		#ifdef FANCY  /////enhance water
		#else
			// Completely insane, but if I don't have these two lines in here, the water doesn't render on a Nexus 6
			float4 surfColor = float4(PSInput.color.rgb, 1.0);
			PSInput.color = surfColor;
		#endif //FANCY

		float cameraDist = cameraDepth / FAR_CHUNKS_DISTANCE;
		float alphaFadeOut = clamp(cameraDist, 0.0, 1.0);
		PSInput.color.a = lerp(PSInput.color.a, 1.0, alphaFadeOut);
	}
#endif
}

// Local Variables:
// mode: hlsl
// indent-tabs-mode: t
// End:
