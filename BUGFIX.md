# bugfixes
This is a file for keeping track of bug fixes, as early as March 23rd of 2022.

## mb101: March 23rd, 2022
Fixed a bug in which type objects would be garbage collected without warning. Now, a type object's reference count is automatically incremented when created to prevent this from happening.
