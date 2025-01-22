SUBDIRS = greeting

process:
	for plugin in $(SUBDIRS); do \
		make -f $(MAINDIR)/plugins/$$plugin/Makefile; \
	done
