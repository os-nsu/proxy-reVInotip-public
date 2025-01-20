greeting: greeting.o
	$(CC) -shared $(MY_LDFLAGS) $(MAIN_DIR)/plugins/greeting/greeting.o -o $(MAIN_DIR)/plugins/greeting/greeting.so

greeting.o:
	$(CC) -I$(INCLUDE_PATH) -fpic -c $(MY_CCFLAGS) $(MAIN_DIR)/plugins/greeting/greeting.c -o $(MAIN_DIR)/plugins/greeting/greeting.o

clean:
	rm -rf *.o *.a *.so
