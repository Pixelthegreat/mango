#include <string.h> /* string functions */
#include <stdio.h> /* file stuff */
#include <stdlib.h> /* memory management */

/* get stuff */
extern char *dfgets(FILE *f) {

	/* vars */
	unsigned int buf_sz = 128; /* buffer sz */
	char *s = (char *)malloc(buf_sz); /* buffer */
	unsigned int i = 0; /* index in buffer */
	int k; /* current character value */

	/* until newline */
	while ((k = fgetc(f)) != '\n') {

		/* resize buf */
		if (i >= buf_sz-1) { /* buf_sz-1 because of null term char */
		
			buf_sz *= 2;
			s = (char *)realloc(s, buf_sz);
		}

		if (k == EOF) {

			/* file error */
			if (i == 0 || !feof(f))
				return NULL;

			break;
		}

		/* store char */
		s[i++] = (char)k;
	}

	/* null term character */
	s[i++] = '\0';

	/* return buffer */
	return s;
}

/* stdin version */
char *dgets() {

	/* return buffer */
	return dfgets(stdin);
}

/* get file extension pointer from filename */
extern char *fngext(char *fn) {

	/* variables */
	int dotcnt = 0; /* dot count */
	int finaldot = 0; /* final dot position */

	/* loop through the string entirely */
	for (int i = 0; i < strlen(fn); i++) {

		/* '.' */
		if (fn[i] == '.') {

			dotcnt++;
			finaldot = i;
		}
	}

	/* no dots */
	if (dotcnt == 0)
		return NULL;

	/* otherwise */
	return &fn[finaldot+1];
}