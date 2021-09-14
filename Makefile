CCFLAGS=-m32

output: main.o object.o error.o names.o token.o node.o file.o lexer.o parser.o bytecode.o stringext.o argparse.o run.o
	gcc $(CCFLAGS) main.o object.o error.o names.o token.o node.o lexer.o parser.o bytecode.o stringext.o argparse.o run.o -o mango

main.o: main.c mango.h
	gcc -c main.c $(CCFLAGS)

object.o: object.c object.h
	gcc -c object.c $(CCFLAGS)

error.o: error.c error.h
	gcc -c error.c $(CCFLAGS)

names.o: names.c names.h
	gcc -c names.c $(CCFLAGS)

token.o: token.c token.h
	gcc -c token.c $(CCFLAGS)

node.o: node.c node.h
	gcc -c node.c $(CCFLAGS)

lexer.o: lexer.c lexer.h
	gcc -c lexer.c $(CCFLAGS)

parser.o: parser.c parser.h
	gcc -c parser.c $(CCFLAGS)

bytecode.o: bytecode.c bytecode.h
	gcc -c bytecode.c $(CCFLAGS)

stringext.o: stringext.c stringext.h
	gcc -c stringext.c $(CCFLAGS)

argparse.o: argparse.c argparse.h
	gcc -c argparse.c $(CCFLAGS)

run.o: run.c run.h
	gcc -c run.c $(CCFLAGS)

clean:
	rm *.o mango
