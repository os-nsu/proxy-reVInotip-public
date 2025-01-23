config: config.o
	ar -rc $(SOURCE)/libconfig.a config.o && ranlib $(SOURCE)/libconfig.a

config.o:
	$(CC) $(MY_CCFLAGS) $(MAIN_DIR)/$(ROOT)/config/config.c