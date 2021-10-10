/* src/shaders/glsl/natural-mystic-config.fxh.  Generated from natural-mystic-config.fxh.in by configure.  */
/* src/shaders/glsl/natural-mystic-config.fxh.in.  Generated from configure.ac by autoheader.  */

#if !defined(NATURAL_MYSTIC_CONFIG_FXH_INCLUDED)
#define NATURAL_MYSTIC_CONFIG_FXH_INCLUDED 1

#define FOG_TYPE_LINEAR 1
#define FOG_TYPE_EXP 2
#define FOG_TYPE_EXP2 3

/* Define to show the fog color. This is not compatible with other debug
   options. */
/* #undef DEBUG_SHOW_FOG_COLOR */

/* Define to show the fog control parameters. This is not compatible with
   other debug options. */
/* #undef DEBUG_SHOW_FOG_CONTROL */

/* Define to show the ambient occlusion factor. This is not compatible with
   other debug options. */
/* #undef DEBUG_SHOW_OCCLUSION_FACTOR */

/* Define to show the terrain-dependent sunlight level in grayscale. This is
   not compatible with other debug options. */
/* #undef DEBUG_SHOW_SUNLIGHT_LEVEL */

/* Define to show the color of vertices. This is not compatible with other
   debug options. */
/* #undef DEBUG_SHOW_VERTEX_COLOR */

/* Define to enable a thin fog that always affects the scene, not only when
   it's raining. */
#define ENABLE_BASE_FOG 1

/* Define to enable highlight and shade on shader-generated clouds. */
#define ENABLE_CLOUD_SHADE 1

/* Define to enable fancy water rendering. */
#define ENABLE_FANCY_WATER 1

/* Define to enable shader-generated clouds. */
#define ENABLE_FBM_CLOUDS 1

/* Define to enable fake shadows generated from the ambient occlusion */
#define ENABLE_OCCLUSION_SHADOWS 1

/* Define to introduce randomness in the brighness of stars. */
#define ENABLE_RANDOM_STARS 1

/* Define to enable water ripple animation that appears on the ground when
   it's raining. */
#define ENABLE_RIPPLES 1

/* Define to enable the shader-generated sun and the moon. */
#define ENABLE_SHADER_SUN_MOON 1

/* Define to enable specular lighting. */
#define ENABLE_SPECULAR 1

/* Define to enable torch light flickering effect. */
#define ENABLE_TORCH_FLICKER 1

/* Define to enable waves of water and leaves. */
#define ENABLE_WAVES 1

/* Define to one of FOG_TYPE_LINEAR and FOG_TYPE_EXP2 to choose a fog type to
   use. */
#define FOG_TYPE FOG_TYPE_EXP2

/* Name of package */
#define PACKAGE "natural-mystic-shaders"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "https://github.com/depressed-pho/natural-mystic-shaders/issues"

/* Define to the full name of this package. */
#define PACKAGE_NAME "Natural Mystic Shaders"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "Natural Mystic Shaders 1.9.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "natural-mystic-shaders"

/* Define to the home page for this package. */
#define PACKAGE_URL "https://github.com/depressed-pho/natural-mystic-shaders"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.9.0"

/* Version number of package */
/* NOTE: A macro of "VERSION" is already used in hlsl shaders, so it had 
   changed to "NUM_VERSION" on GLSL to HLSL convert. */
#define NUM_VERSION "1.9.0"

#endif /* NATURAL_MYSTIC_CONFIG_FXH_INCLUDED */
