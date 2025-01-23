
LIB_DIRS = config logger

all: libs
	$(CC) $(CFLAGS) $(COPT) $(CPPFLAGS) -o proxy $(MAINDIR)/src/backend/master/master.c   $(LDFLAGS) -Wl,-rpath,$(MAINDIR)/install
		mv ./proxy install


libs:
	for target_dir in $(LIB_DIRS); do \
		make -f $(MAINDIR)/src/backend/$$target_dir/Makefile; \
	done