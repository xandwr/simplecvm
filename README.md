# Simple Virtual Machine (SVM) and Assembler

#### Author: Xander Pickering (3118504)

#### Date Updated: 2024/10/07

---

## Overview:

This project contains a simple virtual machine (svm) and an assembler (sasm). The assembler translates assembly language into machine code, which the virtual machine then reads and executes. This project uses a two-pass assembler to handle labels and generate the correct machine code for execution.

### Files:

1. **sasm.c**:
   - **Purpose**: This file implements the assembler for the virtual machine. It reads an assembly source file, performs a two-pass assembly process, and outputs the corresponding machine code.
   - **Key Components**:
     - ```get_register_code()```: Converts a register name to its corresponding machine code.
     - ```write16()```: Writes 16-bit machine code values to standard output.
     - ```strip_comments()```, trim_whitespace(): Preprocessing functions to clean up assembly lines.
     - ```first_pass()```: Builds a symbol table by identifying labels and their memory addresses.
     - ```second_pass()```: Generates the machine code based on the symbol table and instruction set.
     - ```add_label()```, ```find_label()```: Functions for handling labels in the symbol table.
   - **Usage**: The assembler reads an assembly file, processes it into binary machine code, and outputs it.
2. **svm.c**:
   - **Purpose**: Implements the virtual machine that reads machine code (output from sasm) and executes it. The virtual machine simulates a CPU with registers, flags, and a program counter, and can execute various instructions such as LOAD, STORE, ADD, and control flow commands like JMP.
   - **Key Components**:
     - ```fetchImmediate()```: Fetches 16-bit values from memory.
     - ```set_flags()```, ```set_flags_for_load()```: Updates CPU flags (Zero, Negative, Overflow) based on the result of arithmetic or load operations.
     - ```processor_cycle()```: The main loop of the virtual machine that fetches, decodes, and executes instructions.
     - ```load_program()```: Loads machine code into memory.
     - ```initialize_cpu()```: Initializes the CPU registers and flags.
   - **Usage**: After assembling a program using sasm, the machine code is fed to the virtual machine for execution.
3. **svm.h**:
   - **Purpose**: Defines constants and macros for the virtual machine and assembler, such as memory size, opcode values, and register mappings. This file is included in both svm.c and sasm.c.
   - **Key Components**:
     - Opcode definitions for the virtual machine's instruction set (HALT, LOAD, ADD, etc.).
     - Register definitions (R1, R2, A1, A2).
4. **Makefile**
   - **Purpose**: Automates the compilation and testing process for the project.
   - **Key Components**:
     - all: Builds both the assembler (sasm) and the virtual machine (svm) and runs the tests.
     - sasm: Compiles the assembler.
     - svm: Compiles the virtual machine.
     - test: Assembles and runs test programs, compares their output to expected results.
     - clean: Cleans up generated files after testing.
   - **Usage**: Run ```make all``` to compile the assembler and virtual machine and run all tests.
     Run ```make clean``` to remove all generated files.

## Usage:

#### Compiling:

To compile the assembler and virtual machine and automatically clean up generated files, use:

```bash
make all
```

This will create the executables ```sasm``` (assembler) and ```svm``` (virtual machine), run the provided tests with them, and then delete everything back to the default state afterwards.

*NOTE: By default, AUTOCLEAN is set to 1, meaning the files will be automatically cleaned up after testing. If you want to let this data persist, set the AUTOCLEAN flag in the Makefile to 0.*

#### Assembling and Running Programs (when AUTOCLEAN = 0):

To assemble an assembly program (e.g., test.svm), run:

```bash
./sasm < test.svm > test.bin
```

To execute the machine code with the virtual machine, run:

```bash
./svm < test.bin
```

#### Running Tests (when AUTOCLEAN = 0):

Tests are provided in the tests/ directory. To run all tests, use:

```bash
make test
```

The test outputs will be compared to expected results, and discrepancies will be reported.

#### Cleanup (when AUTOCLEAN = 0):

To clean up the project (remove executables and test outputs), use:

```bash
make clean
```

## License

This project is licensed under the MIT License. \
Xander Pickering @ 2024
