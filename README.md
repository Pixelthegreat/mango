# mango
A programming language also written in C.

# updates
this updates section of the README.md file will only show the latest free updates. For more information, please read the README.md file from older commits or go to the releases/tags section of the repository.

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


## Nov 22, 2021 - 0.0.1
The first real update! This update adds the majority of features missing from Mango. The only exceptions are Typedef-ing and Structs. As for the "unsigned" and "const" keywords, i have designed that those would be too unethical and they don't make much sense compared to how much they did in the planning phase for Mango.

Keep in mind that I haven't done much bug testing. As such, from here on, I plan to do extensive bug checks to flush any errors down the drain.

Finally, I should mention that I am designating this update as an alpha release. This means that until the beta stage of 0.x releases, I will try and plan any features that I haven't thought of before that I might want to add. The beta stage will purely focus on bug testing.

Added:
- More stuff for the BIVMT
- The GNU GPLv2 License
- Builtin functions (functions that can be called from Mango, but actually call C code in turn)
- More problems then I can count

Modified:
- Changed a bunch more stuff until I was happy with it (particularly the bytecode compiler)

# standard library

I am planning on including the main code for the standard library in a separate repository. I am also planning on using the previously mentioned MangoLib bytecode format for the standard library.
