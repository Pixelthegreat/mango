/* testing program (if you want to use vim, then don't erase this comment */

struct wbuf -> [
	int fd = 1; /* file descriptor */
	chr *n = "???? mariano's on his way\n"; /* file name */
	int l = 26; /* length of file */
];

wbuf b;
b->fd = 1;
b->l = 14;
b->n = "hello, world!\n";

wbuf c;
c->fd = 1;
c->l = 10;
c->n = "abcdefghi\n";

write(wbuf->fd, b->n, c->l);