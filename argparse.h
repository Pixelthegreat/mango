#ifndef _ARGPARSE_H
#define _ARGPARSE_H

/* flag types */
#define FLAG_COMP_LIB (unsigned int)(0)
#define FLAG_COMP_BIN (unsigned int)(1)
#define FLAG_IDATA (unsigned int)(2)

/* functions */
extern int argparse_set_flag(unsigned int); /* set a flag */
extern int argparse_get_flag(unsigned int); /* get flag state */
extern int argparse_flag2bcm(); /* get bytecode mode from different flags */
extern int argparse(int, char **); /* parse arguments from argc and argv */
extern int argparse_one(char *); /* parse one argument */
extern int argparse_two(char *, char *); /* parse two arguments */
extern char **argparse_filenames(); /* receive pointer to filename list */
extern int argparse_fileslen(); /* get number of names in filename list */
extern void argparse_free(); /* free file name list */

#endif /* _ARGPARSE_H */