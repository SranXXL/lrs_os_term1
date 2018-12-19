CFLAGS = -fsanitize=address

ALL:
	cppcheck main.c
	gcc -o Sobel main.c $(CFLAGS) -lm
