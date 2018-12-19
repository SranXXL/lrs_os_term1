CFLAGS = -fsanitize=address

ALL:
	cppcheck lr5.c
	gcc -D_REENTRANT -o lr5 lr5.c $(CFLAGS) -lm -lpthread
