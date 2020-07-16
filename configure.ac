#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.69])
AC_INIT([Natural Mystic Shaders],
        [1.9.0],
        [https://github.com/depressed-pho/natural-mystic-shaders/issues],
        [],
        [https://github.com/depressed-pho/natural-mystic-shaders])
AM_INIT_AUTOMAKE([1.9])
AC_CONFIG_SRCDIR([src/manifest.json.in])
AC_CONFIG_MACRO_DIR([m4])
m4_pattern_forbid([^AX_])

AM_MAINTAINER_MODE
AM_SILENT_RULES([yes])

# Generate a configuration header. It's intended to be included by
# shaders, not C sources.
AC_CONFIG_HEADERS([src/shaders/glsl/natural-mystic-config.h])
AH_TOP([#if !defined(NATURAL_MYSTIC_CONFIG_H_INCLUDED)
#define NATURAL_MYSTIC_CONFIG_H_INCLUDED 1])
AH_BOTTOM([#endif /* NATURAL_MYSTIC_CONFIG_H_INCLUDED */])

# Checks for options.
AX_MCPACK_INSTALLATION_DIRECTORY([resource])
AX_MCPACK_GENERATE_DIFF_FILE([yes])

AC_ARG_ENABLE(
    [torch-flicker],
    [AS_HELP_STRING(
         [--disable-torch-flicker],
         [disable torch flickering effect])])
AS_IF([test x"$enable_torch_flicker" != x"no"],
      [AC_DEFINE([ENABLE_TORCH_FLICKER], [1],
                 [Define to enable torch light flickering effect.])])

AC_ARG_ENABLE(
    [fancy-water],
    [AS_HELP_STRING(
         [--disable-fancy-water],
         [disable fancy water rendering])])
AS_IF([test x"$enable_fancy_water" != x"no"],
      [AC_DEFINE([ENABLE_FANCY_WATER], [1],
                 [Define to enable fancy water rendering.])])

AC_ARG_ENABLE(
    [waves],
    [AS_HELP_STRING(
         [--disable-waves],
         [disable waves of water and leaves])])
AS_IF([test x"$enable_waves" != x"no"],
      [AC_DEFINE([ENABLE_WAVES], [1],
                 [Define to enable waves of water and leaves.])])

AC_ARG_ENABLE(
    [occlusion-shadows],
    [AS_HELP_STRING(
         [--disable-occlusion-shadows],
         [disable fake shadows generated from the ambient occlusion])])
AS_IF([test x"$enable_occlusion_shadows" != x"no"],
      [AC_DEFINE([ENABLE_OCCLUSION_SHADOWS], [1],
                 [Define to enable fake shadows generated from the ambient occlusion])])

AC_ARG_ENABLE(
    [specular],
    [AS_HELP_STRING(
         [--disable-specular],
         [disable specular lighting])])
AS_IF([test x"$enable_specular" != x"no"],
      [AC_DEFINE([ENABLE_SPECULAR], [1],
                 [Define to enable specular lighting.])])

AC_ARG_ENABLE(
    [ripples],
    [AS_HELP_STRING(
         [--disable-ripples],
         [disable the water ripple animation])])
AS_IF([test x"$enable_ripples" != x"no"],
      [AC_DEFINE([ENABLE_RIPPLES], [1],
                 [Define to enable water ripple animation that appears on the ground when it's raining.])])

AC_ARG_ENABLE(
    [fbm-clouds],
    [AS_HELP_STRING(
         [--disable-fbm-clouds],
         [disable shader-generated clouds])])
AS_IF([test x"$enable_fbm_clouds" != x"no"],
      [AC_DEFINE([ENABLE_FBM_CLOUDS], [1],
                 [Define to enable shader-generated clouds.])])

AC_ARG_ENABLE(
    [cloud-shade],
    [AS_HELP_STRING(
         [--disable-cloud-shade],
         [disable highlight and shade on fbm-clouds])])
AS_IF([test x"$enable_cloud_shade" != x"no"],
      [AC_DEFINE([ENABLE_CLOUD_SHADE], [1],
                 [Define to enable highlight and shade on shader-generated clouds.])])

AC_ARG_ENABLE(
    [random-stars],
    [AS_HELP_STRING(
         [--disable-random-stars],
         [disable randomness in the brightness of stars])])
AS_IF([test x"$enable_random_stars" != x"no"],
      [AC_DEFINE([ENABLE_RANDOM_STARS], [1],
                 [Define to introduce randomness in the brighness of stars.])])

AC_ARG_ENABLE(
    [base-fog],
    [AS_HELP_STRING(
         [--disable-base-fog],
         [disable a thin fog that always affects the scene.])])
AS_IF([test x"$enable_base_fog" != x"no"],
      [AC_DEFINE([ENABLE_BASE_FOG], [1],
                 [Define to enable a thin fog that always affects the scene, not only when it's raining.])])

AC_ARG_ENABLE(
    [shader-sun-moon],
    [AS_HELP_STRING(
         [--disable-shader-sun-moon],
         [disable the shader-generated sun and the moon])])
AS_IF([test x"$enable_shader_sun_moon" != x"no"],
      [AC_DEFINE([ENABLE_SHADER_SUN_MOON], [1],
                 [Define to enable the shader-generated sun and the moon.])])

AC_ARG_WITH(
    [fog-type],
    [AS_HELP_STRING(
         [--with-fog-type=@<:@linear|exp2@:>@],
         [fog type to use. linear: linear fog, exp2: exponential squared fog @<:default: exp2@>:@])])
AH_TOP([#define FOG_TYPE_LINEAR 1
#define FOG_TYPE_EXP 2
#define FOG_TYPE_EXP2 3])
AH_TEMPLATE(
    [FOG_TYPE],
    [Define to one of FOG_TYPE_LINEAR and FOG_TYPE_EXP2 to choose a fog type to use.])
AS_CASE(
    [$with_fog_type],
    ["no"],            [],
    ["linear"],        [AC_DEFINE([FOG_TYPE], [FOG_TYPE_LINEAR])],
    ["exp"],           [AC_DEFINE([FOG_TYPE], [FOG_TYPE_EXP])],
    ["yes"|""|"exp2"], [AC_DEFINE([FOG_TYPE], [FOG_TYPE_EXP2])],
    [AC_MSG_ERROR(
         [Unknown fog type `$with_fog_type'. See `configure --help'.])])

# Debug options.
AH_TEMPLATE(
    [DEBUG_SHOW_VERTEX_COLOR],
    [Define to show the color of vertices. This is not compatible with other debug options.])

AH_TEMPLATE(
    [DEBUG_SHOW_SUNLIGHT_LEVEL],
    [Define to show the terrain-dependent sunlight level in grayscale. This is not compatible with other debug options.])

AH_TEMPLATE(
    [DEBUG_SHOW_OCCLUSION_FACTOR],
    [Define to show the ambient occlusion factor. This is not compatible with other debug options.])

AH_TEMPLATE(
    [DEBUG_SHOW_FOG_COLOR],
    [Define to show the fog color. This is not compatible with other debug options.])

AH_TEMPLATE(
    [DEBUG_SHOW_FOG_CONTROL],
    [Define to show the fog control parameters. This is not compatible with other debug options.])

# Checks for programs.
AC_ARG_VAR([DIFF], [The diff command @<:@autodetected@:>@])
AC_CHECK_PROGS([DIFF], [diff])
AS_IF([test "x$DIFF" = x],
      [AC_MSG_ERROR([You need to install a diff command.])])

AC_ARG_VAR([SED], [The sed command @<:@autodetected@:>@])
AC_PROG_SED
AS_IF([test "x$SED" = x],
      [AC_MSG_ERROR([You need to install a sed command.])])

AC_ARG_VAR([ZIP], [The Info-Zip zip command @<:@autodetected@:>@])
AC_CHECK_PROGS([ZIP], [zip])
AS_IF([test "x$ZIP" = x],
      [AC_MSG_ERROR([You need to install the Info-Zip zip command.])])

AC_ARG_VAR([UNZIP], [The Info-Zip unzip command @<:@autodetected@:>@])
AC_CHECK_PROGS([UNZIP], [unzip])
AS_IF([test "x$UNZIP" = x],
      [AC_MSG_ERROR([You need to install the Info-Zip unzip command.])])

AC_ARG_VAR([INKSCAPE], [The inkscape command @<:@autodetected@:>@])
AC_CHECK_PROGS([INKSCAPE], [inkscape])
AS_IF([test x"$INKSCAPE" = x],
      [AC_MSG_WARN([You need to install Inkscape, or you won't be able to regenerate the pack icon.])
       INKSCAPE=false])

# Checks for libraries.

# Checks for header files.

# Checks for typedefs, structures, and compiler characteristics.

# Checks for library functions.

AC_CONFIG_FILES([
    Makefile
    img/Makefile
    src/Makefile
    orig/Makefile
])
AC_OUTPUT
