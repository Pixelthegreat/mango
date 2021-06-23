# mango
A programming language also written in C.

## reason of creation
A little earlier this year I created Adamite, and I uploaded it to github a few months ago. I thought that I would use some of my spare summer time to create a new language that is similar, but fixes most if not all of the issues with adamite. Of course, all of the problems and issues that I had with adamite would be hard to fix without writing an entirely new language or replacing all of the old adamite code.

## naming scheme
As a disclaimer, I nor anyone I know owns the name "mango" as a copyrighted name or trademark. I don't mean to steal the name or use it without permission, I simply thought of it when thinking of a new project to work on awhile back.

The name has no real meaning, and like adamite, there is no logo or anything to promote the name. Whether it specifically relates to anything that the language has to offer or not, I like the name and I plan to use it so long as it is not copyrighted.

## implementation update 2021-06-23

The parser is now finished (for now), and I have started work on the very beginnings of the bytecode system. For reference, I have included "bytecode_structure2.txt" in the project folder, but it doesn't actually have a lot of information so far. Currently, the bytecode system just prints out the binary information created. However, I do plan on adding in more File I/O stuff soon, so that options can be passed to select a specific file, compile to library or bytecode executable, execute an existing bytecode file, or just execute the code generated immediately. The fairly basic argument system will also include the ability to "link" existing bytecode libraries. As I have mentioned below, I plan on compiling the majority of the standard library compiled to a bytecode library. The only code for the standard library that won't be compiled will be the header files (because I am planning on using a system somewhat similar to C that isn't actually required if you so desire). Of course, you can still use "include" with your required source files for your library if you wish to do something more similar to python modules.

### standard library

I am planning on including the main code for the standard library in a separate repository. I am also planning on using the previously mentioned MangoLib bytecode format for the standard library.

## going further
I am planning on making a new commit every time at least two "sections" are finished. For example, I am working on a brand-new object storage and variable name system as well as an error system, which will probably all come within 3rd commit. I will also make sure to change the README.md file accordingly.