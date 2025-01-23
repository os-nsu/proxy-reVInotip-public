all: greeting.o
	gcc -shared -o greeting.so greeting.o -ldl -L$(MAINDIR)/install/plugins/
	mv greeting.so $(MAINDIR)/install/plugins/
	make clean
greeting.o:
	gcc -c -fPIC $(MAINDIR)/plugins/greeting/greeting.c

clean:
	rm -rf *.so
	rm -rf *.a
	rm -rf *.o