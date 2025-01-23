all: config.o
	ar -rcs libconfig.a config.o
	mv ./libconfig.a install

config.o:
	gcc -c src/backend/config/config.c