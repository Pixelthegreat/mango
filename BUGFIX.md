# bugfixes
This is a file for keeping track of bug fixes, as early as March 23rd of 2022.

## mb101: March 23rd, 2022
Fixed a bug in which type objects would be garbage collected without warning. Now, a type object's reference count is automatically incremented when created to prevent this from happening.

## mb102: April 21st, 2022
Fixed a bug in which whenever an attempt was made to access a struct pointer stored in a struct, Mango would report it as undefined. The cause of this bug stems from the fact that when going down the list of names in a variable access node (i.e., `a->b`), whenever Mango came across a pointer to a struct, it would dereference it; the default value of undefined struct pointers happens to be NULL, which indicates to Mango that it couldn't find the value in the struct.
