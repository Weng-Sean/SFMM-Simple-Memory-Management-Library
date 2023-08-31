# SFMM: Simple Memory Management Library

SFMM (Simple Memory Management Library) is a C library that provides basic memory allocation and management capabilities similar to the `malloc`,
`free`, and `realloc` functions in the standard C library. This library is intended to be a simplified memory allocator for educational purposes
and is not meant to replace professional memory management libraries like the standard C library's `malloc`.

## Features
- **Memory Allocation**: Allocate memory blocks of various sizes.
- **Memory Deallocation**: Free previously allocated memory.
- **Memory Reallocation**: Resize memory blocks.
- **Alignment Support**: Allocate memory with specified alignment.
- **Quick Lists**: Efficiently manage frequently used block sizes.
- **Error Handling**: Detect and handle errors such as invalid pointers.

## Getting Started
### Prerequisites

- A C compiler (e.g., GCC)
- Make sure you have `sfmm.h` and `sfmm.c` included in your project.
### Compilation

To compile your project using the SFMM library, you need to include the `sfmm.c` source file and link it with your program. For example:

```bash
gcc -o my_program my_program.c sfmm.c

