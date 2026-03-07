all: main

main: main.c
	gcc main.c -o main

run: main
	./main

clean:
	rm -f main