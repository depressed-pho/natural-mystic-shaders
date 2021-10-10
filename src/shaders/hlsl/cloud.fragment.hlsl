#include "ShaderConstants.fxh"
#include "natural-mystic-config.fxh"

struct PS_Input
{
    float4 position : SV_Position;
    float4 color : COLOR;
};

struct PS_Output
{
    float4 color : SV_Target;
};

ROOT_SIGNATURE
void main(in PS_Input PSInput, out PS_Output PSOutput)
{
#if defined(ENABLE_FBM_CLOUDS)
    /* We completely disable the vanilla clouds. It's impossible to
     * improve it. Instead we render clouds with sky shaders. */
    discard;
#else
    PSOutput.color = PSInput.color;
#endif /* ENABLE_FBM_CLOUDS */
}

// Local Variables:
// mode: hlsl
// indent-tabs-mode: nil
// End: