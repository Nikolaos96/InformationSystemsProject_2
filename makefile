main: main.o function.o
	gcc main.o function.o -o main

main.o: main.c
	gcc -c main.c

function.o: function.c
	gcc -c function.c

clean:
	rm -f main main.o function.o

run:
	./main -D files -F small.init -Q files/small.work
