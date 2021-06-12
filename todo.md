
## TODO

- Add parsing modes with the syntax `mode;` and `mode;;`
  - Example: `;ksd` is the same as `kill swap duplicate`
    - Note that this uses the parsing mode that has an empty name, its ok
  - Example: `m;+-*/%` is the same as `add sub mul div mod`
    - Note that the `m` parsing mode includes the previous parsing mode
    - So `m;ksd+-*/%` is the same as `kill swap duplicate add sub mul div mod`
- Make it work

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

### Language targets

Add language emitters like Python 3, OCaml, assembly, brainfuck, machine code,
etc. They can be organised as a tree, like the C language being a node and C99,
C11, C11 POSIX, C11 Windows, etc. being children of the C node. Targetting the
C language can be implemented as the compiler choosing a child depending on the
current system.

### Bootstrap

Omg this will be sooo difficult ><'.
