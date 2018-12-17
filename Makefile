JQ ?= jq
zip ?= zip

dist:
	set -eu; \
	: Extract the version from the manifest; \
	ver=`$(JQ) --raw-output '.header.version | map(tostring) | join(".")' src/manifest.json`; \
	cd src; \
	zip -r ../Natural-Mystic-Shaders-$$ver.mcpack *
