export MAIN_DIR = $(CURDIR)
export INCLUDE_PATH = $(MAIN_DIR)/src/include
export SOURCE = $(MAIN_DIR)/install

CC = gcc
MY_CCFLAGS = ${CCFLAGS} ${CFLAGS} ${COPT} ${CPPFLAGS} -fPIC -c -Wall -Wpointer-arith -Wendif-labels -Wmissing-format-attribute -Wimplicit-fallthrough=3 -Wcast-function-type -Wrestrict -Wshadow=compatible-local -Wformat-security -fno-strict-aliasing -fwrapv -g -O2 -I$(INCLUDE_PATH)
MY_LDFLAGS = ${CCFLAGS} ${CFLAGS} ${COPT} ${CPPFLAGS} ${LDFLAGS} -ldl -export-dynamic
MY_LDFLAGS_DEBUG = -fsanitize=address,leak,undefined
ROOT = src/backend
BIN_NAME = proxy
OBJ = plugins_manager.o stack.o
GOALS = plugins_manager utils config
MAIN_OBJ = master.o
LIBS_LINK = -lconfig

all:
	make clean_source_dir
	make create_source_dir
	make link
	make plugin
	echo "Done"

debug:
	make clean_source_dir
	make create_source_dir
	make link_debug
	make plugin
	echo "Done"

link: $(MAIN_OBJ) $(OBJ)
	$(CC) $(MY_LDFLAGS) $^ -L$(SOURCE) $(LIBS_LINK) -Wl,-rpath=$(SOURCE) -o $(SOURCE)/$(BIN_NAME)

link_debug: $(MAIN_OBJ) $(OBJ)
	$(CC) $(MY_LDFLAGS) $(MY_LDFLAGS_DEBUG) $^ -L$(SOURCE) $(LIBS_LINK) -Wl,-rpath=$(SOURCE) -o $(SOURCE)/$(BIN_NAME)

plugin:
	make -f plugins/Makefile

include $(ROOT)/Makefile

create_source_dir:
	mkdir install
	mkdir install/plugins

clean_source_dir:
	rm -rf install

clean_obj:
	rm -rf *.o

clean: clean_obj
	echo "Full clean"