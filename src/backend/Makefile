LIBS_LINK += -lconfig -llogger

SUBDIRS = plugins_manager utils config logger

$(MAIN_OBJ): make_all_subdirs
	$(CC) $(MY_CCFLAGS) $(ROOT)/master.c

make_all_subdirs:
	for subdir in $(SUBDIRS); do\
		make -f $(MAIN_DIR)/$(ROOT)/$$subdir/Makefile;\
	done
