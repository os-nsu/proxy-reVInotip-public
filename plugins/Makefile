SUBDIRS = greeting

make_all_plugins:
	for plugin in $(SUBDIRS); do \
		make -f plugins/$$plugin/Makefile; \
		cp $(MAIN_DIR)/plugins/$$plugin/$$plugin.so $(SOURCE)/plugins/$$plugin.so; \
	done