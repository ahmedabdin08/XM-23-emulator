# XM-23-emulator
This is an emulator for the X-Makina23 machine designed by Dr. Larry Hughes at Dalhousie for the Computer Architecture course.

This machine is a 16-bit RISC architecture based on the ARM Cortex, TI MSP430, and Intel x86 architectures. XM-23 has 41 instructions, eight program-accessible registers (5 general purpose, link register, stack pointer, and program counter), 64 kibs of memory, 32 bytes of cache, and support for up to 8 memory-mapped devices. The machine also handles exceptions such as interrupts from devices, faults, and traps, with their information stored in the interrupt vector table in high memory. The program's status is stored in the program status word.

## CPU
The CPU contains an ALU, 8 program-accessible registers in the register file, a memory buffer/data register (MBR), a memory address register (MAR), an instruction register (IR), and a program status word register (PSW). The CPU has a basic fetch-decode-execute cycle. In fetch, it fetches the instruction from memory and increments the program counter. After fetch, it decodes the instruction onto the opcode and operands. It then executes the instruction and updates the PSW. The CPU has 5 addressing modes: register, direct, indexed, relative, and immediate. It also has eight built-in constants (0, 1, 2, 4, 8, 16, 32, -1) The block diagram for the CPU is seen below

![XM-23 - ISA_Page_008](https://github.com/ahmedabdin08/XM-23-emulator/assets/97932039/f1bbc3b0-bab2-44cd-af84-4bf915e51f75)

### Instruction Set
The XM-23 supports 41 fixed-length instructions and an additional 26 instructions can be emulated are seen below
![XM-23 - ISA_Page_101](https://github.com/ahmedabdin08/XM-23-emulator/assets/97932039/85e6fcc3-bc46-4c3f-a1bd-36dba21b904b)
![XM-23 - ISA_Page_050](https://github.com/ahmedabdin08/XM-23-emulator/assets/97932039/1c240685-97a6-477f-82c1-f95ccf16cc3c)

### Machine state
The PSW holds information regarding the state of the machine such as the carry bit if the last instruction resulted in a carry, a fault bit if the CPU is executing a fault handler, its current priority, etc. The PSW's bits can be seen below
![23 05 09-10 - XM3 ISA_Page_22](https://github.com/ahmedabdin08/XM-23-emulator/assets/97932039/5b2833a7-c41a-4863-97f3-9f39f8bc9af7)

##Memory
The XM23 has 64 kibs of primary memory and 32 bytes of cache memory. Memory is organized in a little-endian format where the least significant byte comes first and the most significant byte comes last in 16-bit word pairs. Primary memory is accessed through the bus. Memory address 0x0000 to 0x000F contain the data, control, and status of the devices; while memory addresses 0xFFC0 to 0xFFFF contain the interrupt vector table for handling interrupts, faults, and traps. There is also the cache used to speed up access to instructions and data. There are two cache policies implemented for writes: write back and write through. Finally, there are three cache organiztion methods implemented: fully associative, direct mapping, and N-way associative.

## Devices
The XM-23 has support for up to 8 devices. These devices include a control/status register (CSR), a data register (DR), and a corresponding entry in the interrupt vector table if interrupts are enabled. The CSR bits and DR can be seen below
![23 07 11 - Exceptions P1_Page_10](https://github.com/ahmedabdin08/XM-23-emulator/assets/97932039/9ae20014-eb20-4e04-acc4-5cb07333e5ce)

The devices can either be accessed by polling or through the use of interrupts. The machine is supplied with three devices: a programmable timer, keyboard, and screen.

## Exceptions

The machine handles three types of exceptions interrupts, faults, and traps, with their information being stored in the interrupt vector table.
![23 07 13 - Exceptions P2_Page_11](https://github.com/ahmedabdin08/XM-23-emulator/assets/97932039/a37de604-4cf0-4665-9ef7-805133d7e830)
![23 07 13 - Exceptions P2_Page_14](https://github.com/ahmedabdin08/XM-23-emulator/assets/97932039/b6469357-0166-4ddb-8e15-b5df89f68253)

Whenever an exception occurs the program counter (PC), link register (LR), PSW, and CEX state are pushed onto the stack. The PSW is updated with the value of the PSW within the corresponding interrupt vector entry's PSW and the previous priority is updated, the PC changes to the address within the vector entry, and the LR is assigned a value of 0xFFFF. The LR is modified to 0xFFFF is cause there is no return from interrupt and the PC returns to its value by being assigned the LR invalid value of 0xFFFF.

### Interrupts
Interrupts are signaled by a device when a status change occurs.Through the use of multi-level queues the machine also supports tail chainning in which moving from one interrupt handler to another has no overhead of pushing and pulling from the stack.

### Faults
The XM23 has four faults that are supported: illegal instruction, invalid address, priority fault, and double fault. An illegal instruction fault happens when the CPU tries to decode or execute an instruction with an invalid opcode. An invalid address fault occurs when the CPU tries to fetch an instruction at an odd address. A priority fault occurs either when the application tries to increase its priority or when the application makes a supervisor call to a trap handler with a lower priority. A double fault is a fault within a fault.

### Traps
Traps are software interrupts in which the application performs a system/supervisor call to the operating system to perform a task. They are done in the emulator through the use of SVC instruction.
