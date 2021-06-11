
output: main.o object.o error.o names.o
	gcc -m32 main.o object.o error.o names.o -o main

main.o: main.c
	gcc -c main.c -m32

object.o: object.c object.h
	gcc -c object.c -m32

error.o: error.c error.h
	gcc -c error.c -m32

names.o: names.c names.h
	gcc -c names.c -m32

clean:
	rm *.o main