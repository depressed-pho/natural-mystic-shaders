SUBDIRS = src orig

EXTRA_DIST = \
	LICENSE \
	README.md

AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

.PHONY: fixme
fixme:
	@grep -n -E '(FIXME|THINKME|TODO)' --color=always -r $(SUBDIRS:%=$(srcdir)/%) || \
		echo 'No FIXME, THINKME, nor TODO were found.'
