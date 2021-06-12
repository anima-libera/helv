
## TODO

- Make a preprocessor that supports
  - Macro definition
  - Macro expansion
- Make it work

- Add file inclusion support for the preprocessor
- Make a mini stdlib to test it
- Make it work

- Make each instruction having its own C emitting function
- Make it work

- These instruction C emitting functions all take a `modes_t*`
- The emitting engine updates a `modes_t` according to the passing mode changes
- The emitting core code is called the *emitting engine* (fancy)
- And the code handling modes is called the *mode system* (fancyyy)
- Add the modes `64` and `8`
- Make it work

- Add ways to manually push and pop `modes_t`
  - Like blocks that push a copy of the top `modes_t` and pop it at the end
  - Or directives to manipulate the `modes_t` stack
- Make it work

- Add more instructions
  - Add an instruction to push the stack height
  - Add instructions to set/get a value in the stack given its index
  - Add rotations of the top N values in both directions
  - Add many more handful instructions

- Actually, add more functions to each instruction
  - The emitting engine should do more passes
  - The first ones are for listing all the needed features
  - They call functions on instructions that just say what they will need
- Add a feature that uses that
- Make it work

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

### Optimizations

Make an IR (Intermediary Representation) that decomposes elementary
instructions into micro instructions that can interact with a stack and
registers. It must support operands that can be from the stack to also can be
immediate values.
Then, make a data flow graph thing, and use it to optimize things.
Examples of wanted optimisations:
- `push_8 kill` optimized into `nop`
- `push_8 call_pop` optimized into `call_8`

Also, handle register allocation, it will allow serious optimisations for any
assemby backend support thing.

### Language targets

Add language emitters like Python 3, OCaml, assembly, brainfuck, machine code,
etc. They can be organised as a tree, like the C language being a node and C99,
C11, C11 POSIX, C11 Windows, etc. being children of the C node. Targetting the
C language can be implemented as the compiler choosing a child depending on the
current system.

### Bootstrap

Omg this will be sooo difficult ><'.
