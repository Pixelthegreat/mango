#include <stdio.h>
#include <stdlib.h>

#include "object.h"
#include "error.h"
#include "names.h"

int main(int argc, char **argv) {

	object *myStr = objectNewString("hello, world!");
	
	objectWrite(FD_CONSOLE, myStr);
	printf("\n");

	if (errorIsSet()) {

		errorPrint();
		errorClear();
	}

	nameTable *nt = namesNew();

	object *o = namesGetFromString(nt, objectNewString("myStr"));

	if (errorIsSet()) {

		errorPrint();
		errorClear();
	}

	namesFree(nt);
	objectFreeAll();
}