logger: logger.o
	$(CC) -shared $(MY_LDFLAGS) logger.o -o $(SOURCE)/liblogger.so

logger.o:
	$(CC) $(MY_CCFLAGS) $(MAIN_DIR)/$(ROOT)/logger/logger.c