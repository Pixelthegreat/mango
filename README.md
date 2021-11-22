# mango
A programming language also written in C.

# updates
this updates section of the README.md file will only show the latest free updates. For more information, please read the README.md file from older commits or go to the releases/tags section of the repository.

Nov 22, 2021 - 0.0.1
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

Oct 30, 2021
Added:
- Bytecode Interpreter Virtual Machine Thing (BIVMT)
- Context System for individual files, libraries, and scopes to contain their own variables.

Modified:
- Completely rewrote the object system from scratch. Now I am happy with it.

# standard library

I am planning on including the main code for the standard library in a separate repository. I am also planning on using the previously mentioned MangoLib bytecode format for the standard library.
