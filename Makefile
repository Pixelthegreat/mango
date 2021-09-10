
output: main.o object.o error.o names.o token.o node.o file.o lexer.o parser.o bytecode.o stringext.o argparse.o run.o
	gcc -m32 main.o object.o error.o names.o token.o node.o lexer.o parser.o bytecode.o stringext.o argparse.o run.o -o main

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

lexer.o: lexer.c lexer.h
	gcc -c lexer.c -m32

parser.o: parser.c parser.h
	gcc -c parser.c -m32

bytecode.o: bytecode.c bytecode.h
	gcc -c bytecode.c -m32

stringext.o: stringext.c stringext.h
	gcc -c stringext.c -m32

argparse.o: argparse.c argparse.h
	gcc -c argparse.c -m32

run.o: run.c run.h
	gcc -c run.c -m32

clean:
	rm *.o main
