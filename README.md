# mango
A programming language also written in C.

## reason of creation
A little earlier this year I created Adamite, and I uploaded it to github a few months ago. I thought that I would use some of my spare summer time to create a new language that is similar, but fixes most if not all of the issues with adamite. Of course, all of the problems and issues that I had with adamite would be hard to fix without writing an entirely new language or replacing all of the old adamite code.

## naming scheme
As a disclaimer, I nor anyone I know owns the name "mango" as a copyrighted name or trademark. I don't mean to steal the name or use it without permission, I simply thought of it when thinking of a new project to work on awhile back.

The name has no real meaning, and like adamite, there is no logo or anything to promote the name. Whether it specifically relates to anything that the language has to offer or not, I like the name and I plan to use it so long as it is not copyrighted.

## implementation update 2021-06-17
I am finished with the start of the parser and I am (pretty much) done with the lexer as well. On top of that, I have a good idea for future syntax. I am working on the standard library, as well as a basic syntax example program. I also have a plan for the bytecode compiler and interpreter.

### bytecode stuff

For bytecode, I will be making my own system to handle it. You should have the ability to run the bytecode generated immediately, or save it to a file in the form of two bytecode file types: MangoBC (Executable form, extension is ".mc"), and MangoLib (Non-executable library form, extension is ".ml"). The only difference between the two is the header information in the beginning of each file.

### standard library

I am planning on including the main code for the standard library in a separate repository. I am also planning on using the previously mentioned MangoLib bytecode format for the standard library.

## going further
I am planning on making a new commit every time at least two "sections" are finished. For example, I am working on a brand-new object storage and variable name system as well as an error system, which will probably all come within 3rd commit. I will also make sure to change the README.md file accordingly.