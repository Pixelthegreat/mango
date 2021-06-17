#include "file.h"
#include <stdlib.h>
#include <stdio.h> /* FILE */
#include <string.h> /* strcat, strlen */

/* variables */
unsigned int current_fd = 2;
file **files = NULL; /* file list */
unsigned int n_of_files = 0; /* number of files in list */
unsigned int file_cap = 0; /* capacity of file list */

/* open a new file */
extern file *fileNew(char *fname, unsigned int mode) {

	/* malloc new file */
	file *f = (file *)malloc(sizeof(file));

	/* values */
	f->fname = fname;
	f->fd = current_fd++;
	f->mode = mode;
	f->is_open = 1;
	f->f_pos = 0;
	f->buffer_cap = 256;
	f->at_eof = 0;
	f->first_char = '\0';

	/* malloc list */
	f->buffer = (char *)malloc(f->buffer_cap);

	for (int i = 0; i < f->buffer_cap; i++)
		f->buffer[i] = '\0';

	/* file mode in string form */
	const char *c_mode;

	if (mode == FILE_MODE_READ)
		c_mode = "r";
	else if (mode == FILE_MODE_WRITE)
		c_mode = "w";
	else
		c_mode = "r";

	/* open file */
	f->f = fopen(fname, c_mode);

	/* if file is not open */
	if (f->f == NULL)
		f->is_open = 0;

	/* put file in list */
	if (files == NULL) {

		files = (file **)malloc(sizeof(file *) * 8);
		file_cap = 8;
		n_of_files = 0;
	}

	if (n_of_files >= file_cap) {

		file_cap *= 2;
		files = (file **)realloc(files, sizeof(file *) * file_cap);
	}

	files[n_of_files++] = f;

	/* return file */
	return f;
}

/* read text into a file (at max, buffer size is read) */
extern void fileRead(file *f) {

	/* reset buffer */
	for (int i = 0; i < f->buffer_cap; i++)
		f->buffer[i] = '\0';

	/* loop through and read chars */
	unsigned int idx = 0; /* position relative to buffer */
	while ((!f->at_eof) && idx < f->buffer_cap - 1) {

		/* read char */
		fileReadChar(f, idx);
		idx++;
	}
}

/* read all the contents of a file */
extern char *fileReadAll(file *f) {

	/* create a buffer */
	int buf_sz = 1024;
	char *buf = (char *)malloc(buf_sz);
	strcpy(buf, "");

	/* read text */
	while (!f->at_eof) {

		/* read buffer */
		fileRead(f);

		/* resize buffer */
		if (strlen(buf) + strlen(f->buffer) + 2 >= buf_sz) {

			buf_sz *= 2;
			buf = (char *)realloc(buf, buf_sz);
		}

		/* concatenate strings */
		strcat(buf, f->buffer);
	}

	/* return buffer */
	return buf;
}

/* read a char into a specific position in buffer */
extern void fileReadChar(file *f, unsigned int idx) {

	/* get next char */
	int c = fgetc(f->f);
	f->at_eof = (c == EOF); /* end of file */
	if (!f->at_eof) f->buffer[idx] = (char)c; /* store char */

	/* store first char */
	if (f->f_pos == 0)
		f->first_char = f->buffer[idx];

	f->f_pos++; /* advance pos */
}

/* seek to a position */
extern void fileSeek(file *f, unsigned int pos) {

	/* set position and seek */
	f->f_pos = pos;
	fseek(f->f, pos, SEEK_SET);
}

/* reset the buffer and seek to start pos */
extern void fileReset(file *f) {

	fileSeek(f, 0); /* seek */
	for (int i = 0; i < f->buffer_cap; i++)
		f->buffer[i] = '\0';
}

/* get a file with a descriptor */
extern file *fileGet(unsigned int fd) {

	/* loop through list */
	for (int i = 0; i < n_of_files; i++)
		if (files[i]->fd == fd)
			return files[i];

	return NULL; /* default */
}

/* close a file */
extern void fileClose(file *f) {

	if (f->f != NULL) {
	
		fclose(f->f); /* close file if needed */
		f->f = NULL; /* remove file pointer */
	}

	f->is_open = 0; /* closed now */
}

/* free a file */
extern void fileFree(file *f) {

	/* close the file */
	fileClose(f);

	/* free buffer and file */
	free(f->buffer);
	free(f);
}

/* free all files and list of files */
extern void fileFreeAll() {

	/* loop */
	for (int i = 0; i < n_of_files; i++)
		fileFree(files[i]);

	/* free list */
	free(files);
}