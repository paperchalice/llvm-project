# LLVM MMIX backend

## Introduction
> MMIX is a computer intended to illustrate machine-level aspects of programming. In my books The Art of Computer Programming,
  it replaces MIX, the 1960s-style machine that formerly played such a role...
  I strove to design MMIX so that its machine language would be simple, elegant, and easy to learn.
  At the same time I was careful to include all of the complexities needed to achieve high performance in practice,
  so that MMIX could in principle be built and even perhaps be competitive with some of the fastest general-purpose computers in the marketplace."
>
> -- Donald Ervin Knuth

## Instruction selection
Here we use the latest (up to 2023) instruction framework --- Global Instruction Selection.
### Calling convention handling
We use global instruction selection here, the calling convention handling is a
problem. Under the framework of global instruction selection, calling convention
is handled before instruction selection as well as register allocation.
The calling convention used by DEK is very casual, it uses the first registers
as much as possible, and use PUSH-like instructions to save them, so the
`callLowering` is hard to determine how many registers will be pushed into the stack,
so here I set the value `X` be 15, i.e. force push the first 16 registers to stack.
In future, may implement a real DEK calling convention by implementing a custom
register allocator, but how to model this problem is also a problem, if use graph
coloring, the constraint in this situation is somewhat weaker than pre-color...
