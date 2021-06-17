
output: main.o object.o error.o names.o token.o node.o file.o lexer.o parser.o
	gcc -m32 main.o object.o error.o names.o token.o node.o file.o lexer.o parser.o -o main

main.o: main.c mango.h
	gcc -c main.c -m32

object.o: object.c object.h
	gcc -c object.c -m32

error.o: error.c error.h
	gcc -c error.c -m32

names.o: names.c names.h
	gcc -c names.c -m32

token.o: token.c token.h
	gcc -c token.c -m32

node.o: node.c node.h
	gcc -c node.c -m32

file.o: file.c file.h
	gcc -c file.c -m32

lexer.o: lexer.c lexer.h
	gcc -c lexer.c -m32

parser.o: parser.c parser.h
	gcc -c parser.c -m32

clean:
	rm *.o main