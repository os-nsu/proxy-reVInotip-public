SUBDIRS = greeting

make_all_plugins:
	for plugin in $(SUBDIRS); do \
		make -f plugins/$$plugin/Makefile; \
		mkdir $(SOURCE)/plugins/$$plugin; \
		touch $(SOURCE)/plugins/$$plugin/$$plugin.so && cp $(MAIN_DIR)/plugins/$$plugin/$$plugin.so $(SOURCE)/plugins/$$plugin/$$plugin.so; \
	done