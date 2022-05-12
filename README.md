# mango
A programming language also written in C.

# updates
this updates section of the README.md file will only show the latest three updates. For more information, please read the README.md file from older commits or go to the releases/tags section of the repository.

## May 12, 2022 - 0.2.1
This update is a feature and bugfix update. I am working on language documentation, although I have yet to release it.

Added:
- The `argv` array which contains arguments passed after `--` in the command line, as well `argc` for the number of arguments
- The `none` and `nullptr` constants to act as struct pointers to nothing; however, they cannot be dereferenced without error
- The `else` statement finally works; example:

```
if (condition) -> [

	dostuff();
	
] else dootherstuff();
```

Fixed:
- mb102: Fixed a bug in which whenever an attempt was made to access a struct pointer stored in a struct, Mango would report it as undefined

I am working on a new language called Emerald, and I hope to have the first version with all of my originally planned features available before the next Mango update; however, Mango will still be worked on into the future.

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

# standard library

I am planning on including the main code for the standard library in a separate repository. I am also planning on using the previously mentioned MangoLib bytecode format for the standard library.
