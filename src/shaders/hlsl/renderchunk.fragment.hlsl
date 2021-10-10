#include "ShaderConstants.fxh"
#include "util.fxh"
#include "natural-mystic-color.fxh"
#include "natural-mystic-config.fxh"
#include "natural-mystic-fog.fxh"
#include "natural-mystic-hacks.fxh"
#include "natural-mystic-light.fxh"
#include "natural-mystic-rain.fxh"
#include "natural-mystic-water.fxh"

struct PS_Input
{
	float4 position : SV_Position;

#ifndef BYPASS_PIXEL_SHADER
	lpfloat4 color : COLOR;
	snorm float2 uv0 : TEXCOORD_0_FB_MSAA;
	snorm float2 uv1 : TEXCOORD_1_FB_MSAA;
#endif

/* Workaround for https://bugs.mojang.com/browse/MCPE-40059 */
#if defined(MCPE40059)
float3 wPos : wPos;
float cameraDist : camDist;
float3 vNormal : vNormal;
float flickerFactor : flickerFactor;
float desatFactor : desatFactor;
float clearWeather : clearWeather;
float waterFlag : waterFlag;
float waterPlane : waterPlane;
#endif

#ifdef FOG
	float4 fogColor : FOG_COLOR;
#endif
};

struct PS_Output
{
	float4 color : SV_Target;
};

ROOT_SIGNATURE
void main(in PS_Input PSInput, out PS_Output PSOutput)
{
#ifdef BYPASS_PIXEL_SHADER
    PSOutput.color = float4(0.0f, 0.0f, 0.0f, 0.0f);
    return;
#else

#if USE_TEXEL_AA
	float4 diffuse = texture2D_AA(TEXTURE_0, TextureSampler0, PSInput.uv0 );
#else
	float4 diffuse = TEXTURE_0.Sample(TextureSampler0, PSInput.uv0);
#endif

#ifdef SEASONS_FAR
	diffuse.a = 1.0f;
#endif

#if USE_ALPHA_TEST
	#ifdef ALPHA_TO_COVERAGE
		#define ALPHA_THRESHOLD 0.05
	#else
		#define ALPHA_THRESHOLD 0.5
	#endif
	if(diffuse.a < ALPHA_THRESHOLD)
		discard;
#endif

#if defined(BLEND)
	diffuse.a *= PSInput.color.a;
#endif

#ifndef SEASONS
	#if !USE_ALPHA_TEST && !defined(BLEND)
		diffuse.a = PSInput.color.a;
	#endif	

	diffuse.rgb *= PSInput.color.rgb;

	/* The game appears to encode an ambient occlusion in the vertex
	 * color in this case. Abuse it to create more shadows. */
	float occlusion = occlusionFactor(PSInput.color.rgb);
#else
	float2 uv = PSInput.color.xy;
	diffuse.rgb *= lerp(1.0f, TEXTURE_2.Sample(TextureSampler2, uv).rgb*2.0f, PSInput.color.b);
	diffuse.rgb *= PSInput.color.aaa;
	diffuse.a = 1.0f;

	float occlusion = 1.0; /* Assume it's not occluded at all. */
#endif

	/* Fetch the level of daylight (i.e. the one which darkens at
	 * night) from the light map passed by the upstream. Note that we
	 * intentionally reduce the dynamic range because the upstream
	 * daylight level doesn't drop to zero at night. */
	float daylight = TEXTURE_1.Sample(TextureSampler1, float2(0.0, 1.0)).r;
	daylight = smoothstep(0.4, 1.0, daylight);

	/* Fetch the level of ambient light from the light map passed by
	 * the upstream. The constant multiplifier is determined so the
	 * intensity will be 6 on the Overworld and 26 in the
	 * Nether/End. */
	float ambientBrightness = TEXTURE_1.Sample(TextureSampler1, float2(0.0, 0.0)).r * 44.797;

	/* Calculate the color of the ambient light based on the fog
	 * color. We'll use it at several places. Note that the .a
	 * component denotes the intensity. */
#if defined(UNDERWATER)
	static const bool isUnderwater = true;
#else
	static const bool isUnderwater = false;
#endif /* defined(UNDERWATER) */

	float3 ambientColor;
#if defined(FOG) && defined(MCPE40059)
	if (isRenderDistanceFog(FOG_CONTROL)) {
		/* Don't use the fog color in this case, as it would be
		 * slightly different from the color of near terrain. */
		ambientColor = ambientLightColor(PSInput.uv1.y, daylight);
	}
	else {
		/* The existence of bad weather fog (and also underwater fog)
		 * should increase the intensity of ambient light (#32). But
		 * at night it should work the other way.
		 */
		if (isUnderwater) {
			ambientColor = ambientLightColor(PSInput.fogColor);
			ambientBrightness *= lerp(0.9, 1.4, daylight);
		}
		else {
			ambientColor = lerp(
				ambientLightColor(PSInput.fogColor),
				ambientLightColor(PSInput.uv1.y, daylight),
				PSInput.clearWeather);
			ambientBrightness *= lerp(lerp(0.9, 1.4, daylight), 1.0, PSInput.clearWeather);
		}
	}
#else
	ambientColor = ambientLightColor(PSInput.uv1.y, daylight);
#endif /* defined(FOG) */

	/* Save the diffused color here as the color of the material. We
	 * are going to redo all the lightings with our own HDR method. */
	float3 pigment = diffuse.rgb;

	/* Accumulate all the light to one linear RGB floattor. We are going
	 * to use it for diffuse lighting, and also specular lighting. */
	float3 dirLight   = 0.0;
	float3 undirLight = 0.0;

	undirLight += ambientLight(ambientColor, ambientBrightness);
#if defined(FOG) && defined(MCPE40059)
	/* When it's raining the sunlight shouldn't affect the scene
	 * (#24), but we cannot treat the rain as a boolean switch as that
	 * would cause #40. */
	if (isUnderwater) {
		dirLight += sunlight(PSInput.uv1.y, daylight);
		dirLight += moonlight(PSInput.uv1.y, daylight);
	}
	else {
		dirLight += sunlight(PSInput.uv1.y, daylight) * PSInput.clearWeather;
		dirLight += moonlight(PSInput.uv1.y, daylight) * PSInput.clearWeather;
	}
#else
	dirLight += sunlight(PSInput.uv1.y, daylight);
	dirLight += moonlight(PSInput.uv1.y, daylight);
#endif /* FOG */
	undirLight += skylight(PSInput.uv1.y, daylight);
#if defined(MCPE40059)
	/* Torchlight is directional, but since we don't actually know
	 * their directions we have to consider it as undirectional. */
	undirLight += torchLight(PSInput.uv1.x, PSInput.uv1.y, daylight, PSInput.flickerFactor);
#endif

	/* Light sources should be significantly brighter than regular
	 * objects. */
#if defined(ALWAYS_LIT)
	undirLight += emissiveLight(PSInput.flickerFactor);
#endif

	/* Now we finished accumulating light. Compute the diffuse light
	 * and the specular light here. We assume the color of specular
	 * light is always the same as the color of accumulated light. */
#if defined(MCPE40059)
	float3  sNormal = normalize(cross(ddx(-PSInput.wPos), ddy(PSInput.wPos)));
	float   wet     = wetness(PSInput.clearWeather, PSInput.uv1.y);
	if (PSInput.waterFlag > 0.5) {
		/* Compute the specular light and the opacity of water. It is
		 * tempting to do this only when defined(BLEND), but if we do
		 * that water in far terrain will have different colors.
		 *
		 * We need a per-fragment normal here, which is an
		 * interpolated vertex normal passed by our vertex
		 * shader. However, we can't compute it correctly for anything
		 * other than flat top surfaces because the game doesn't tell
		 * us anything about normals. In the fragment shader we can
		 * compute the surface normal with ddx/ddy but it's only a
		 * normal of flat triangles. Luckily for us as the water
		 * volume decreases the perturbance also decreases and thus
		 * the surface becomes more and more flat. So to disguise the
		 * problem we lerp the flat surface normal with the smooth,
		 * analytical but the incorrect one, using the volume. */
		float3 fNormal  = normalize(lerp(sNormal, PSInput.vNormal, PSInput.waterPlane));

#  if defined(ENABLE_WAVES)
		/* Perturb the normal even more, but this time with much higher
		 * frequencies. This is a kind of bump mapping. */
		static const float distThreshold = 0.6;
		static const float distFadeStart = distThreshold * 0.8;
		if (PSInput.cameraDist < distThreshold) {
			/* But perturbing the normal on far geometry doesn't
			 * contribute to the overall quality, and it may even
			 * cause aliasing. Also reduce the perturbance depending
			 * on the sunlight level. */
			float3 perturbed = waterWaveNormal(PSInput.wPos, TOTAL_REAL_WORLD_TIME, fNormal);
			perturbed = lerp(perturbed, fNormal,
							smoothstep(distFadeStart, distThreshold, PSInput.cameraDist));
			perturbed = lerp(sNormal, perturbed, smoothstep(0.5, 1.0, PSInput.uv1.y));
			fNormal   = normalize(perturbed);
		}
#  endif /* defined(ENABLE_WAVES) */

		diffuse.rgb  = pigment * (dirLight + undirLight);
		diffuse.rgb *= 0.5; // Darken the base water color.

#  if defined(ENABLE_SPECULAR)
		float4 specular = waterSpecularLight(diffuse.a, dirLight, undirLight, PSInput.wPos, TOTAL_REAL_WORLD_TIME, fNormal);

		diffuse.rgb += specular.rgb;
		diffuse.a    = specular.a;
#  endif /* defined(ENABLE_SPECULAR) */

#  if defined(ENABLE_RIPPLES)
		if (wet > 0.0) {
			diffuse.rgb += wet * ripples(dirLight + undirLight, PSInput.wPos, PSInput.cameraDist, TOTAL_REAL_WORLD_TIME, fNormal);
		}
#  endif /* defined(ENABLE_RIPPLES) */
	}
	else {
#  if defined(ENABLE_OCCLUSION_SHADOWS)
		/* The intensity of directional light should be affected by the
		 * occlusion factor. */
		static const float occlShadow = 0.35;
		dirLight *= lerp(occlShadow, 1.0, occlusion);
#  endif /* defined(ENABLE_OCCLUSION_SHADOWS) */

		/* Wet ground should have reduced diffuse light if it's made
		 * of a rough material. But for now it's a constant value
		 * because we don't know what a material it's made of. */
		diffuse.rgb = pigment * (dirLight + undirLight);
		diffuse.rgb *= lerp(1.0, 0.5, wet);

#  if defined(ENABLE_SPECULAR)
		static const float fresnel   = 0.04;
		static const float shininess = 2.0;
		float3 specular = specularLight(fresnel, shininess, dirLight, undirLight, PSInput.wPos, sNormal);

		diffuse.rgb *= 1.0 - fresnel;
		diffuse.rgb += specular * lerp(1.0, 5.0, wet);
#  endif /* defined(ENABLE_SPECULAR) */

#  if defined(ENABLE_RIPPLES)
		if (wet > 0.0) {
			diffuse.rgb += wet * ripples(dirLight + undirLight, PSInput.wPos, PSInput.cameraDist, TOTAL_REAL_WORLD_TIME, sNormal);
		}
#  endif /* defined(ENABLE_RIPPLES) */
	}
#endif /* defined(MCPE40059) */

	diffuse.rgb = uncharted2ToneMap(diffuse.rgb, 112.0, 1.0);
	diffuse.rgb = contrastFilter(diffuse.rgb, 1.25);

	/* Reduce the contrast of far objects (#5). The overall color
	 * should lean towards the ambient. And we also apply a permanent
	 * fog. It should ideally be biome-specific (#9) but we can't do
	 * that currently, so we always use the ambient color. */
#if defined(ENABLE_BASE_FOG) && defined(MCPE40059)
	static const float contrast   = 0.45;
	static const float fogDensity = 0.4;
	diffuse.rgb = lerp(
		diffuse.rgb,
		lerp(contrastFilter(diffuse.rgb, contrast) * ambientColor,
			fogBrightness(PSInput.uv1.x, PSInput.uv1.y, daylight) * ambientColor,
			fogDensity),
		PSInput.desatFactor);
#endif

	/* We can't apply fogs before doing tone mapping, because that
	 * changes the fog color and breaks "render distance fog". */
#ifdef FOG
	if (isUnderwater || isRenderDistanceFog(FOG_CONTROL)) {
		diffuse.rgb = lerp( diffuse.rgb, PSInput.fogColor.rgb, PSInput.fogColor.a );
	}
	else {
		/* During a bad weather everything should look dull. We do a
		 * desaturation for that (#6). The reason for the lerp() is so
		 * fogs not only reduce saturation but also contrast.
		 */
		diffuse.rgb = lerp(
			desaturate(diffuse.rgb, PSInput.fogColor.a) * ambientColor,
			PSInput.fogColor.rgb,
			PSInput.fogColor.a);
	}
#endif

#if defined(DEBUG_SHOW_VERTEX_COLOR)
	diffuse = PSInput.color;

#elif defined(DEBUG_SHOW_SUNLIGHT_LEVEL)
	diffuse.rgb = PSInput.uv1.y;

#elif defined(DEBUG_SHOW_OCCLUSION_FACTOR)
	diffuse.rgb = occlusion;

#elif defined(DEBUG_SHOW_FOG_COLOR)
	diffuse.rgb = FOG_COLOR.rgb;

#elif defined(DEBUG_SHOW_FOG_CONTROL) && defined(MCPE40059)
	diffuse.rgb = cameraDist < 0.5 ? FOG_CONTROL.x : FOG_CONTROL.y;

#endif /* DEBUG */

	PSOutput.color = diffuse;

#ifdef VR_MODE
	// On Rift, the transition from 0 brightness to the lowest 8 bit value is abrupt, so clamp to 
	// the lowest 8 bit value.
	PSOutput.color = max(PSOutput.color, 1 / 255.0f);
#endif

#endif // BYPASS_PIXEL_SHADER
}

// Local Variables:
// mode: hlsl
// indent-tabs-mode: t
// End: