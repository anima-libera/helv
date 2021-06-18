
## TODO (not in order or anyting)

- Make a REPL
- Make it work

- Make it so that `;;` opens and close the short mode
  - It must error if it is not closed on the same block level thing
    - `uwu [ awa ;; owo [ [ iwi ] ywy ] h ;; h h [ ] ]` ok
    - `uwu [ awa ;; owo [ [ iwi ] ywy ] h ] [ ;; h h [ ] ]` error
    - No open in already open, `;;[;; ... ;;];;` error
  - Is it ok to `;` in a `;;` zone? Idk..
- Make it work

- Make words for basic instruction
- Add more basic instructions
  - Add a dowhile loop
  - Add a loop that pops n and then pops f and then exec f n times
  - Add ifelse
  - Add the 5 arithmetic operators
  - etc.
- Make it work

- Support the `-d` debug command line argument
- Maybe like a verbose mode?

- Make a preprocessor that supports
  - Macro definition (with parameters)
  - Macro expansion (with arguments)
- Make it work

- Add file inclusion support for the preprocessor
- Make a mini stdlib to test it
- Make it work

- Add more instructions
  - Add an instruction to push the stack height
  - Add instructions to set/get a value in the stack given its index
    - From top or bottom
  - Add rotations of the top N values in both directions
    - And why not of the N bottom values too
  - Add many more handful instructions

- Make a real readme

- Add test suite features
- Make a test suite
- Make it work

## Ideas

### Interpreter, compile time execution and REPL

Use the curly braces `{ }` for the syntax that enters and exit compile time
execution code. With such a feature, the preprocessor would be all mighty!

This needs the compiler to have an interpreter too. Each instruction should
have a function that modifies an execution context. The interpreter should be
made available in a REPL ^^.

### Modes

Static modes have the syntax `mode,` for referring to the next element, and
`mode,,` for referring to all the following elements until said otherwise.

Note: maybe not this syntax actually, we will see.

Dynamic modes have.. some syntax, for emitting code that will change the
variables that describe the dynamic mode involved.

Some dynamic modes will need the help of some static modes to work, for example
if a dynamic mode that changes the size of the head (like `8` or `64`) is used,
the size of the head has to be stored somewhere during the execution of the
section of code that uses this dynamic mode, so... This may be a bad example
actually.

Actually, here is how it should work: some static modes are used to indicate
that a section of the code uses some dynamic mode, and in such a section the
appropriate dynamic modes can be changed.

### Allocator

The compiler provides only very few low-level allocation-related instructions,
and the stdlib defines ways that can easily allocate stacks(!).

Then the stdlib could provide ways to move the head to one of these allocated
stacks. Could be nice ^^.

### Optimizations of the compiled program

Make an IR (Intermediate Representation) that decomposes elementary
instructions into micro instructions that can interact with a stack and
registers. It must support operands that can be from the stack to also can be
immediate values.
Then, make a data flow graph thing, and use it to optimize things.
Examples of wanted optimisations:
- `push_8 kill` optimized into `nop`
- `push_8 call_pop` optimized into `call_8`

Also, handle register allocation, it will allow serious optimisations for any
assemby backend support thing.

### Optimizations of the compiler

Lets keep that for the far future, don't forget what Knuth-sama once said about
early optimizations.

### Language targets

Add language emitters like Python 3, OCaml, assembly, brainfuck, machine code,
etc. They can be organised as a tree, like the C language being a node and C99,
C11, C11 POSIX, C11 Windows, etc. being children of the C node. Targetting the
C language can be implemented as the compiler choosing a child depending on the
current system.

### Bootstrap

Omg this will be sooo difficult ><'.
