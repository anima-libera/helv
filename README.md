
# Helv - A stack-based compiled programming language

Note: The project is still in very early alpha 0.0.0 dev developpement,
for now the best README might actually be
all the files of the repo except `README.md`.

## Inspired by

Helv is inspired by
[FALSE](http://strlen.com/false-language/) and
[FORTH](https://www.forth.com/starting-forth/),
two intresting languages to any stack-based programming enthusiast.

## So it is compiled huh?

Currently, it """compiles""" to C. Compiling more like transpiling >﹏<.
Yes, yes, I know, but *real* compilation will (hopefully) be supported one day.

## The Great Plan

- Get dat nice lil low-level stack-based lang to compile to C
- Interpret Helv
- Add more backends like assembly, machine code, LLVM IR, Python, etc.
- Add high-level cool features (but still stack-based somehow)
  - When it can be done, do some crazy stuff in the standard library
- Optimize the compiler, the interpreter, the compiled programs, etc.
- Support for cool stuff like the DWARF debug format, and other... formats?
- Support embedding the compiler/interpreter in C programs
- Support embedding compiled programs
- Support embedding the compiler in compiled programs (when C is targetted)
- Bootstrap
- Support doing crazy stuff with the Helv implementation in Helv
- Make a logo in inkscape, open a discord server, get a userbase, for profit
- IDE support

Note: This Great Plan doesn't have a particular order..

## IDE support

For now, all you get is some primitive syntax highlighting for Sublime Text.
Still better than nothing!

## Wanna contribute?

If you can't stand the absolute lack of features/support/things Helv-related,
then sure feel free to contribute, I'll be very happy ^^.
All of you will be *extremely* welcome to even look at this repo haha,
if ya crazy ya can open issues, throw pull requests, get in touch, etc.
and I'll be overjoyed by the simple fact that someone actually gives a shit.
Even if it is all just for pointing out a typo in a comment.

Please note that even if this project is only *for fun*,
I'm still serious about it ^^ !

## The syntax, the specification, the standard

This part of the README will be added when
the syntax will be likely to be stable.

One thing to know is what a `[ ]` block actually do.
It is an instruction that pushes a number on the stack,
and the pushed number is so that it *refers* to a program,
the said program is the code in the `[ ]` block.
When an instruction says it *executes* that number,
you got it: it executes the program refered by the said number.

The number pushed by a `[ ]` block is
the number of `[` that are in the source code
up to (and including) the `[` of the block,
for example `[][][[][]];x` pushes 1, 2, 4 and 5.
The number 0 refers to the whole program itself,
for example `0;x` is an infinite loop (that may stack overflow).

## Why this not in Rüst?!1

I said

> this project is only *for fun*

C is more fun than Rust

Note: Chasing bugs is fun, fighting the borrow checker isn't
(even if the borrow checker helps get the project done quicker, I don't care).

Programming can be a way to craft software, or it can be a way to have fun
(and maybe get a piece of software to be crafted in the process of having fun).

## The main is too long, this is poor design, undefind behavior! Shame!

Sorry ><'

Please refer to the *Wanna contribute?* section of this README.

## Testing

### Interpreter

```sh
python3 _comp.py -d -l ../examples/test.hv -e
```

### "Compiler"

```sh
python3 _comp.py -d -l ../examples/test.hv
```

### Anything else

```sh
python3 _comp.py -d -l --help
```
