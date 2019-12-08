main: main.o function.o sort_join.o join_list.o mid_list.o
	gcc main.o function.o sort_join.o join_list.o mid_list.o -o main

main.o: main.c
	gcc -c main.c

function.o: function.c
	gcc -c function.c

sort_join.o: sort_join.c
	gcc -c sort_join.c

join_list.o: join_list.c
	gcc -c join_list.c

mid_list.o: mid_list.c
	gcc -c mid_list.c

clean:
	rm -f main main.o function.o sort_join.o join_list.o mid_list.o

run:
	./main -D files -F small.init -Q files/small.work

run2:
	./main -D /tmp/workloads/medium/ -F /tmp/workloads/medium/medium.init -Q /tmp/workloads/medium/medium.work
