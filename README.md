# mango
A programming language also written in C.

# updates
this updates section of the README.md file will only show the latest three updates. For more information, please read the README.md file from older commits or go to the releases/tags section of the repository.

## March 29, 2022 - 0.2.0
This update is a feature and bugfix update. Bugs are now listed in BUGFIX.md, so for more information on a bug, please read that. Bugfix numbers are formatted like this, `mb<bug_number>`, and will be referred to in the list of fixed bugs if there are any.

This update adds an API for using libc's builtin `libdl` functions. Although I do not have any examples prepared, I do wish to provide some soon. I also wish to provide general language documentation soon, as well.

Added:
	- The `libdl` API for Mango

Fixed:
	- mb101: Fixed a bug in which type objects would be garbage collected without warning

## February 17, 2022 - 0.1.2
This update is a simple bugfix.

Modified:
	- replaced the regular Makefile system with [my-configure](https://github.com/Pixelthegreat/my-configure). See `./configure.sh -help` for more details, or take a look at the repository for `my-configure`.

Fixed:
	- A bug in which a struct instance would always refer to the same nametable as it's parent (I don't have any clue how this happened, but I fixed it :D)

## December 11, 2021 - 0.1.1
The last update of 2021, in preparation for the Christmas break. This update finalizes the feature set by adding structs (although they aren't in a fully working state), as well as the functionality for '&' and '\*' which are for *address of* and *value of* pointer operations, respectively.

Happy Xmas

Added:
	- Structs (not in a fully-working state yet)
	- '&' and '\*'
Modified:
	- The names system, I only realized a few days ago just how broken it is

# standard library

I am planning on including the main code for the standard library in a separate repository. I am also planning on using the previously mentioned MangoLib bytecode format for the standard library.
