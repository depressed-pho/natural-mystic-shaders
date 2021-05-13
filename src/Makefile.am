MANIFEST_SRC = manifest.json.in

MCPACK_INSTALLATION_DIRECTORY = $(MCPACK_INSTALLATION_DIRECTORY_resource)
MCPACK_FILES = \
	manifest.json \
	materials/common.json \
	materials/entity.material \
	materials/fancy.json \
	materials/fancy.material \
	materials/particles.material \
	materials/sad.json \
	materials/sad.material \
	materials/sky.material \
	materials/terrain.material \
	pack_icon.png \
	shaders/glsl/cloud.fragment \
	shaders/glsl/cloud.vertex \
	shaders/glsl/natural-mystic-cloud.h \
	shaders/glsl/natural-mystic-color.h \
	shaders/glsl/natural-mystic-config.h \
	shaders/glsl/natural-mystic-fog.h \
	shaders/glsl/natural-mystic-hacks.h \
	shaders/glsl/natural-mystic-light.h \
	shaders/glsl/natural-mystic-noise.h \
	shaders/glsl/natural-mystic-precision.h \
	shaders/glsl/natural-mystic-rain.h \
	shaders/glsl/natural-mystic-water.h \
	shaders/glsl/particles.fragment \
	shaders/glsl/particles.vertex \
	shaders/glsl/rain_snow.fragment \
	shaders/glsl/renderchunk.fragment \
	shaders/glsl/renderchunk.vertex \
	shaders/glsl/sky.fragment \
	shaders/glsl/sky.vertex \
	shaders/glsl/stars.vertex \
	shaders/glsl/sun_moon.fragment \
	shaders/glsl/sun_moon.vertex \
	shaders/glsl/uniformPerFrameConstants.h \
	splashes.json

MCPACK_EXTRA_FILES = \
	$(top_srcdir)/LICENSE

EXTRA_DIST = \
	natural-mystic-logo.svg

pack_icon.png: natural-mystic-logo.svg
	$(AM_V_GEN)
	$(AM_V_at)$(INKSCAPE) --export-png="$@" --export-width=128 --export-height=128 "$<"

CLEANFILES=
noinst_DATA=
include $(top_srcdir)/am/manifest.am
include $(top_srcdir)/am/mcpack.am
