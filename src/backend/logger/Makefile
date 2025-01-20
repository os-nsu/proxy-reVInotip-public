logger.o:
	$(CC) $(MY_CCFLAGS) $(ROOT)/logger/logger.c

logger: logger.o
	$(CC) -shared $(MY_LDFLAGS) logger.o -o $(SOURCE)/liblogger.so