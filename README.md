# XM-23-emulator
This is an emulator for the X-Makina23 machine designed by Dr. Larry Hughes at Dalhousie for the Computer Architecture course.

This machine is a 16-bit RISC architecture based on the ARM Cortex, TI MSP430, and Intel x86 architectures. XM-23 has 41 instructions, eight registers (5 general purpose, link register, stack pointer, and program counter), 64 kibs of memory, 32 bytes of cache, and support for up to 8 memory-mapped devices. The machine also handles exceptions such as interrupts from devices, faults, and traps, with their information stored in the interrupt vector table in high memory. The program's status is stored in the program status wrod

## Instruction Set
The XM-23 supports 41 fixed length instructions and an additional 26 instructions can be emulated.
![XM-23 - ISA_Page_101](https://github.com/ahmedabdin08/XM-23-emulator/assets/97932039/85e6fcc3-bc46-4c3f-a1bd-36dba21b904b)
![XM-23 - ISA_Page_050](https://github.com/ahmedabdin08/XM-23-emulator/assets/97932039/1c240685-97a6-477f-82c1-f95ccf16cc3c)
