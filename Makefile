# add -g flag
CCFLAGS = -g
# for libdlopen
LDFLAGS = -ldl

# cross compile
ifdef CROSS_COMPILE
	CC = $(CROSS_COMPILE)gcc
else
	CC = gcc
endif

all: mango

mango: main.o object.o error.o names.o token.o node.o file.o lexer.o parser.o bytecode.o stringext.o argparse.o run.o context.o vm.o
	$(CC) $(CCFLAGS) $(LDFLAGS) main.o object.o error.o names.o token.o node.o lexer.o parser.o bytecode.o stringext.o argparse.o run.o context.o vm.o -o mango

main.o: main.c mango.h
	$(CC) -c main.c $(CCFLAGS)

object.o: object.c object.h intobject.h arrayobject.h structobject.h functionobject.h
	$(CC) -c object.c $(CCFLAGS)

error.o: error.c error.h
	$(CC) -c error.c $(CCFLAGS)

names.o: names.c names.h
	$(CC) -c names.c $(CCFLAGS)

token.o: token.c token.h
	$(CC) -c token.c $(CCFLAGS)

node.o: node.c node.h
	$(CC) -c node.c $(CCFLAGS)

lexer.o: lexer.c lexer.h
	$(CC) -c lexer.c $(CCFLAGS)

parser.o: parser.c parser.h
	$(CC) -c parser.c $(CCFLAGS)

bytecode.o: bytecode.c bytecode.h
	$(CC) -c bytecode.c $(CCFLAGS)

stringext.o: stringext.c stringext.h
	$(CC) -c stringext.c $(CCFLAGS)

argparse.o: argparse.c argparse.h
	$(CC) -c argparse.c $(CCFLAGS)

run.o: run.c run.h
	$(CC) -c run.c $(CCFLAGS)

context.o: context.c context.h
	$(CC) -c context.c $(CCFLAGS)

vm.o: vm.c vm.h
	$(CC) -c vm.c $(CCFLAGS)

clean:
	rm *.o mango

install:
	sudo cp -v mango /usr/bin/

uninstall:
	sudo rm -v /usr/bin/mango
