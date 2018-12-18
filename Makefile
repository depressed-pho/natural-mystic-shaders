JQ ?= jq
zip ?= zip

dist:
	set -eu; \
	: Extract the version from the manifest; \
	ver=`$(JQ) --raw-output '.header.version | map(tostring) | join(".")' src/manifest.json`; \
	mcpack="Natural-Mystic-Shaders-$$ver.mcpack"; \
	rm -f "$$mcpack"; \
	zip "$$mcpack" LICENSE; \
	cd src; \
	zip -r "../$$mcpack" *
