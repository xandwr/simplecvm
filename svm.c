/*
 * svm.c -- Simple Virtual Machine Implementation
 * Author: Xander Pickering (3118504)
 * Updated: 2024/10/07
 *
 * Implements the virtual machine logic for executing instructions.
 * This virtual machine reads machine code from standard input and executes it.
 */

#include "svm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// CPU structure definition
typedef struct {
  uint16_t REG1, REG2;   // Data registers
  uint16_t ADDR1, ADDR2; // Address registers
  uint16_t PC;           // Program counter
  uint8_t Z, N, O;       // Flags (Z = Zero, N = Negative, O = Overflow)
} CPU;

// Memory array
uint8_t memory[MEMORY_SIZE];

// Global CPU state
CPU cpu;

/**
 * Fetches a 16-bit immediate value from memory at the given address.
 *
 * @param address The memory address to read from.
 * @return The 16-bit value fetched from memory.
 */
uint16_t fetchImmediate(uint16_t address) {
  if (address + 1 >= MEMORY_SIZE) {
    fprintf(stderr, "Memory access out of bounds at address %04x\n", address);
    exit(1);
  }
  return (memory[address] << 8) | memory[address + 1];
}

/**
 * Sets the CPU flags (Zero, Negative, Overflow) based on the result of an
 * operation.
 *
 * @param operand1 The first operand.
 * @param operand2 The second operand.
 * @param result The result of the arithmetic operation.
 * @param operation The desired operation to check against.
 */
void set_flags(uint16_t operand1, uint16_t operand2, uint16_t result,
               char operation) {
  // Set Zero flag (Z)
  cpu.Z = (result == 0);

  // Set Negative flag (N)
  cpu.N = (result & 0x8000) != 0; // Check sign bit for 16-bit integer

  // Set Overflow flag (O)
  switch (operation) {
  case '+': // Addition overflow
    if (((operand1 & 0x8000) ==
         (operand2 & 0x8000)) && // Signs of both operands are the same
        ((result & 0x8000) !=
         (operand1 & 0x8000))) // Result's sign differs from the operands' sign
    {
      cpu.O = 1;
    } else {
      cpu.O = 0;
    }
    break;

  case '-': // Subtraction overflow
    if (((operand1 & 0x8000) !=
         (operand2 & 0x8000)) && // Signs of operands differ
        ((result & 0x8000) !=
         (operand1 &
          0x8000))) // Result's sign differs from the first operand's sign
    {
      cpu.O = 1;
    } else {
      cpu.O = 0;
    }
    break;

  default:
    cpu.O = 0; // No overflow by default
  }
}

/**
 * Sets the CPU flags (Zero, Negative, Overflow) based on load settings.
 *
 * @param value The value to set.
 */
void set_flags_for_load(uint16_t value) {
  // Set Zero flag (Z)
  cpu.Z = (value == 0);

  // Set Negative flag (N)
  cpu.N = (value & 0x8000) != 0; // Check sign bit for 16-bit integer
}

/**
 * Executes instructions in a loop until a HALT instruction is encountered.
 */
void processor_cycle() {
  while (1) {
    uint16_t start_PC = cpu.PC; // Save current PC for debugging
    uint16_t immediate;
    int jump = 0;

    // Fetch the opcode
    uint8_t opcode = memory[cpu.PC++];
    // printf("\nPC: %04x, Opcode: %02x, Jump to: %04x (jump=%d, Z=%d, N=%d,
    // O=%d)\n",
    //        cpu.PC, opcode, immediate, jump, cpu.Z, cpu.N, cpu.O);

    /* for (int i = 0; i < 16; i++) {
      printf("Memory[%04x] = %02x\n", i, memory[i]);
    } */

    switch (opcode) {
    case HALT: {
      return;
    }

    case LOAD: {
      uint8_t reg = memory[cpu.PC++];
      immediate = fetchImmediate(cpu.PC);
      cpu.PC += 2;

      if (reg == R1) {
        cpu.REG1 = immediate;
        set_flags_for_load(cpu.REG1);
      } else if (reg == R2) {
        cpu.REG2 = immediate;
        set_flags_for_load(cpu.REG2);
      } else if (reg == A1) {
        cpu.ADDR1 = immediate;
      } else if (reg == A2) {
        cpu.ADDR2 = immediate;
      }
      break;
    }

    case LOADI: {
      uint8_t reg_byte = memory[cpu.PC++];
      uint8_t reg2 = (reg_byte >> 6) & 0x03; // Bits 7-6
      uint8_t reg1 = reg_byte & 0x03;        // Bits 1-0

      uint16_t address;
      if (reg2 == A1) {
        address = cpu.ADDR1;
      } else if (reg2 == A2) {
        address = cpu.ADDR2;
      } else if (reg2 == R1) {
        address = cpu.REG1;
      } else if (reg2 == R2) {
        address = cpu.REG2;
      } else {
        fprintf(stderr, "Invalid address register in LOADI: %d\n", reg2);
        exit(1);
      }

      uint16_t value = fetchImmediate(address);

      if (reg1 == R1) {
        cpu.REG1 = value;
        set_flags_for_load(cpu.REG1);
      } else if (reg1 == R2) {
        cpu.REG2 = value;
        set_flags_for_load(cpu.REG2);
      } else if (reg1 == A1) {
        cpu.ADDR1 = value;
      } else if (reg1 == A2) {
        cpu.ADDR2 = value;
      } else {
        fprintf(stderr, "Invalid destination register in LOADI: %d\n", reg1);
        exit(1);
      }
      break;
    }

    case STORE: {
      uint8_t reg = memory[cpu.PC++];
      immediate = fetchImmediate(cpu.PC);
      cpu.PC += 2;

      uint16_t value = (reg == R1) ? cpu.REG1 : cpu.REG2;
      memory[immediate] = (value >> 8) & 0xFF;
      memory[immediate + 1] = value & 0xFF;
      break;
    }

    case STOREI: {
      uint8_t reg_byte = memory[cpu.PC++];
      uint8_t reg2 = (reg_byte >> 6) & 0x03;
      uint8_t reg1 = reg_byte & 0x03;

      uint16_t address;
      if (reg2 == A1) {
        address = cpu.ADDR1;
      } else if (reg2 == A2) {
        address = cpu.ADDR2;
      } else if (reg2 == R1) {
        address = cpu.REG1;
      } else if (reg2 == R2) {
        address = cpu.REG2;
      } else {
        fprintf(stderr, "Invalid address register in STOREI: %d\n", reg2);
        exit(1);
      }

      uint16_t value;
      if (reg1 == R1) {
        value = cpu.REG1;
      } else if (reg1 == R2) {
        value = cpu.REG2;
      } else if (reg1 == A1) {
        value = cpu.ADDR1;
      } else if (reg1 == A2) {
        value = cpu.ADDR2;
      } else {
        fprintf(stderr, "Invalid source register in STOREI: %d\n", reg1);
        exit(1);
      }

      memory[address] = (value >> 8) & 0xFF;
      memory[address + 1] = value & 0xFF;
      break;
    }

    case ADD: {
      uint8_t reg = memory[cpu.PC++];
      immediate = fetchImmediate(cpu.PC);
      cpu.PC += 2;

      u_int16_t old_value = (reg == R1) ? cpu.REG1 : cpu.REG2;

      if (reg == R1) {
        cpu.REG1 += immediate;
        set_flags(old_value, immediate, cpu.REG1, '+');
      } else if (reg == R2) {
        cpu.REG2 += immediate;
        set_flags(old_value, immediate, cpu.REG2, '+');
      }
      break;
    }

    case ADDR: {
      uint8_t reg_byte = memory[cpu.PC++];
      uint8_t reg2 = (reg_byte >> 6) & 0x03;
      uint8_t reg1 = reg_byte & 0x03;

      uint16_t *dest_reg = (reg1 == R1) ? &cpu.REG1 : &cpu.REG2;
      uint16_t src_value = (reg2 == R1) ? cpu.REG1 : cpu.REG2;
      uint16_t old_value = *dest_reg;

      *dest_reg += src_value;
      set_flags(old_value, src_value, *dest_reg, '+');
      break;
    }

    case SUB: {
      uint8_t reg = memory[cpu.PC++];
      immediate = fetchImmediate(cpu.PC);
      cpu.PC += 2;

      uint16_t old_value = (reg == R1) ? cpu.REG1 : cpu.REG2;

      if (reg == R1) {
        cpu.REG1 -= immediate;
        set_flags(old_value, immediate, cpu.REG1, '-');
      } else if (reg == R2) {
        cpu.REG2 -= immediate;
        set_flags(old_value, immediate, cpu.REG2, '-');
      }
      break;
    }

    case SUBR: {
      uint8_t reg_byte = memory[cpu.PC++];
      uint8_t reg2 = (reg_byte >> 6) & 0x03;
      uint8_t reg1 = reg_byte & 0x03;

      uint16_t *dest_reg = (reg1 == R1) ? &cpu.REG1 : &cpu.REG2;
      uint16_t src_value = (reg2 == R1) ? cpu.REG1 : cpu.REG2;
      uint16_t old_value = *dest_reg;

      *dest_reg -= src_value;
      set_flags(old_value, src_value, *dest_reg, '-');
      break;
    }

    case JMP:
    case JMPZ:
    case JMPN:
    case JMPO: {
      // Take up that pesky extra 1 byte >:)
      uint8_t unused = memory[cpu.PC++];

      if (cpu.PC >= MEMORY_SIZE) {
        fprintf(stderr, "Jumped to invalid memory address %04x\n", cpu.PC);
        exit(1);
      }

      immediate = fetchImmediate(cpu.PC);
      cpu.PC += 2;

      jump = 0;
      if (opcode == JMP)
        jump = 1;
      else if (opcode == JMPZ && cpu.Z)
        jump = 1;
      else if (opcode == JMPN && cpu.N)
        jump = 1;
      else if (opcode == JMPO && cpu.O)
        jump = 1;

      if (jump) {
        if (immediate < MEMORY_SIZE) {
          cpu.PC = immediate;
        } else {
          fprintf(stderr, "Jump to invalid memory: %04x\n", immediate);
          exit(1);
        }
      }
        // Free up that byte
        memset(&unused, 0, sizeof(unused));
      break;
    }

    case OUT: {
      cpu.PC++; // Skip unused byte
      immediate = fetchImmediate(cpu.PC);
      cpu.PC += 2;

      printf("%d", (int16_t)immediate);
      break;
    }

    case OUTC: {
      cpu.PC++; // Skip unused byte
      immediate = fetchImmediate(cpu.PC);
      cpu.PC += 2;

      printf("%c", (uint8_t)(immediate & 0xFF));
      break;
    }

    case OUTR: {
      uint8_t reg = memory[cpu.PC++];
      if (reg == R1) {
        printf("%d", (int16_t)cpu.REG1);
      } else if (reg == R2) {
        printf("%d", (int16_t)cpu.REG2);
      }
      break;
    }

    case OUTRC: {
      uint8_t reg = memory[cpu.PC++];
      if (reg == R1) {
        printf("%c", cpu.REG1 & 0xFF);
      } else if (reg == R2) {
        printf("%c", cpu.REG2 & 0xFF);
      }
      break;
    }

    case OUTI: {
      uint8_t reg = memory[cpu.PC++];
      uint16_t address = (reg == A1) ? cpu.ADDR1 : cpu.ADDR2;
      uint16_t value = fetchImmediate(address);

      printf("%d", (int16_t)value);
      break;
    }

    case OUTIC: {
      uint8_t reg = memory[cpu.PC++];
      uint16_t address = (reg == A1) ? cpu.ADDR1 : cpu.ADDR2;
      uint8_t value = memory[address];

      printf("%c", value);
      break;
    }

    default: {
      fprintf(stderr, "Unknown opcode: %02x at PC = %04x\n", opcode, start_PC);
      exit(1);
    }
    }
  }
}

/**
 * Loads machine code from standard input into memory.
 */
void load_program() {
  uint16_t address = 0;
  int byte;

  while ((byte = getchar()) != EOF && address < MEMORY_SIZE) {
    memory[address++] = (uint8_t)byte;
  }
}

/**
 * Initializes the CPU state.
 */
void initialize_cpu() {
  cpu.PC = 0;
  cpu.REG1 = cpu.REG2 = 0;
  cpu.ADDR1 = cpu.ADDR2 = 0;
  cpu.Z = cpu.N = cpu.O = 0;
}

/**
 * Main function of the virtual machine.
 *
 * @return Exit status code.
 */
int main() {
  // Pre-allocate the needed memory to prevent overflows
  memset(memory, 0, sizeof(memory));

  initialize_cpu();

  // Load program into memory
  load_program();

  // Start the processor cycle
  processor_cycle();

  return 0;
}
