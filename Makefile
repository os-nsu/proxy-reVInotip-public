export MAINDIR = $(CURDIR)

COPT := ${COPT} -g
CFLAGS := ${CFLAGS} -export-dynamic
LDFLAGS := ${LDFLAGS} -L./install/ -ldl -lconfig -llogger
CPPFLAGS := ${CPPFLAGS}

CC = gcc

SUBDIRS = src/backend plugins

all:clean_install prepare_dir backend

	$(CC) $(CFLAGS) $(COPT) $(CPPFLAGS) -o proxy src/backend/master/master.c   $(LDFLAGS) -Wl,-rpath,$(MAINDIR)/install
	mv ./proxy install
	make custom_plugins
	make clean


prepare_dir:
	mkdir install
	mkdir install/plugins

backend:
	make -f src/backend/config/Makefile
	make -f src/backend/logger/Makefile
	mv ./libconfig.a install
	mv ./liblogger.so install

clean:
	rm -rf *.o
	rm -rf *.so
	rm -rf *.a

clean_install:
	rm -rf install

custom_plugins:
	make -f plugins/Makefile