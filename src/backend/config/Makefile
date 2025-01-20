config.o:
	$(CC) $(MY_CCFLAGS) $(ROOT)/config/config.c

config: config.o
	ar -rc $(SOURCE)/libconfig.a config.o && ranlib $(SOURCE)/libconfig.a