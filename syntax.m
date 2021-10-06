/* a comment */
int x = 10 ; /* integer */
chr *s = "hello, world!"; /* string */
ss y = "hello, world!"; /* also a string */

extern int z = 10; /* anything in an "extern" statement always happens in the global scope */

/* function declaration */
fun int function( int x, int y, int z );

/* function definition */
fun int function( int x, int y, int z ) -> [

	return x * (y + z);
];

/* both */
fun int add( int a, int b ) -> [

	return a + b;
];

/* if statement */
if (x == 10) -> [

	x = 11;
];

/* for loop */
for ( int i = 0; i < 8; i = i + 1; ) -> [

	write(0, 'aaaa'); /* print */
];

/* while loop */
while (1) -> [

	read(0, 'aaaa');
];

/* struct */
struct animal -> [
	chr *name;
];

/* get item and set item */
a[x] = b;
b = a[x];
