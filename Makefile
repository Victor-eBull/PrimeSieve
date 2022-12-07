all: main.c
	gcc -o a main.c -O0 -pthread -lm -g

run: a
	./a

fast: main.c
	gcc -o a main.c -O2 -pthread -lm -g

debug: a
	gdb a -r

clean:
	rm -f a out.txt