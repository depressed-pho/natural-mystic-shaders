MANIFEST_SRC = manifest.json.in

MCPACK_INSTALLATION_DIRECTORY = $(MCPACK_INSTALLATION_DIRECTORY_resource)
MCPACK_FILES = \
	manifest.json \
	materials/sky.material \
	pack_icon.png \
	shaders/glsl/cloud.vertex \
	shaders/glsl/natural-mystic-config.h \
	shaders/glsl/natural-mystic-hacks.h \
	shaders/glsl/natural-mystic-utils.h \
	shaders/glsl/renderchunk.fragment \
	shaders/glsl/renderchunk.vertex \
	shaders/glsl/sky.fragment \
	shaders/glsl/sky.vertex \
	shaders/glsl/uniformPerFrameConstants.h \
	splashes.json

EXTRA_DIST = \
	natural-mystic-logo.svg

pack_icon.png: Makefile natural-mystic-logo.svg
	$(AM_V_GEN)
	$(AM_V_at)$(INKSCAPE) --export-png="$@" --export-width=128 --export-height=128 \
		"$(srcdir)/natural-mystic-logo.svg"

CLEANFILES=
noinst_DATA=
include $(top_srcdir)/am/manifest.am
include $(top_srcdir)/am/mcpack.am
