/*
 * sasm.c -- Simple Assembler for the Virtual Machine
 * Author: Xander Pickering (3118504)
 * Updated: 2024/10/07
 *
 * Assembles assembly code into machine code for the virtual machine.
 * Performs a two-pass assembly to handle labels and generate correct machine
 * code.
 */

#include "svm.h"
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LABELS 256

/**
 * Structure to hold label information for the symbol table.
 */
typedef struct {
  char label[MAX_LINE_LENGTH];
  uint16_t address;
} Label;

Label symbol_table[MAX_LABELS];
int label_count = 0;

/**
 * Converts a register name to its encoded value.
 *
 * @param reg The register name (e.g., "R1", "A2").
 * @return The encoded register value, or 0xFF if invalid.
 */
uint8_t get_register_code(const char *reg) {
  if (strcmp(reg, "R1") == 0)
    return 1;
  if (strcmp(reg, "R2") == 0)
    return 0;
  if (strcmp(reg, "A1") == 0)
    return 3;
  if (strcmp(reg, "A2") == 0)
    return 2;

  return 0xFF; // Error value (invalid register)
}

/**
 * Writes a 16-bit value to stdout in binary format.
 *
 * @param value The 16-bit value to write.
 */
void write16(uint16_t value) {
  putchar((value >> 8) & 0xFF);
  putchar(value & 0xFF);
}

/**
 * Strips comments from a line of code.
 *
 * @param line The line to process.
 */
void strip_comments(char *line) {
  char *comment_start = strchr(line, '#');
  if (comment_start) {
    *comment_start = '\0'; // Terminate the line at the start of the comment
  }
}

/**
 * Trims leading and trailing whitespace from a string.
 *
 * @param str The string to trim.
 */
void trim_whitespace(char *str) {
  // Trim leading whitespace
  while (isspace((unsigned char)*str)) {
    memmove(str, str + 1, strlen(str));
  }

  // Trim trailing whitespace
  char *end = str + strlen(str) - 1;
  while (end > str && isspace((unsigned char)*end)) {
    *end = '\0';
    end--;
  }
}

/**
 * Adds a label to the symbol table.
 *
 * @param label The label name.
 * @param address The memory address associated with the label.
 */
void add_label(const char *label, uint16_t address) {
  if (label_count >= MAX_LABELS) {
    fprintf(stderr, "Symbol table overflow.\n");
    exit(1);
  }
  strcpy(symbol_table[label_count].label, label);
  symbol_table[label_count].address = address;
  label_count++;
}

/**
 * Finds a label in the symbol table.
 *
 * @param label The label name to find.
 * @param address Pointer to store the found address.
 * @return 1 if found, 0 otherwise.
 */
int find_label(const char *label, uint16_t *address) {
  for (int i = 0; i < label_count; i++) {
    if (strcmp(symbol_table[i].label, label) == 0) {
      *address = symbol_table[i].address;
      return 1;
    }
  }
  return 0;
}

/**
 * First pass of the assembler: builds the symbol table.
 *
 * @param lines The array of code lines.
 * @param line_count The number of lines.
 * @param instruction_addresses Array to store instruction addresses.
 */
void first_pass(char lines[][MAX_LINE_LENGTH], int line_count,
                uint16_t *instruction_addresses) {
  uint16_t location_counter = 0;

  for (int i = 0; i < line_count; i++) {
    char line_copy[MAX_LINE_LENGTH];
    strcpy(line_copy, lines[i]);
    strip_comments(line_copy);
    trim_whitespace(line_copy);

    if (strlen(line_copy) == 0)
      continue;

    char label[MAX_LINE_LENGTH] = {0};
    char rest_of_line[MAX_LINE_LENGTH] = {0};

    // Check for label (labels are at the beginning of a line and end with a
    // space)
    if (sscanf(line_copy, "%s %[^\n]", label, rest_of_line) == 2) {
      // If the first word is not an instruction, it's a label
      if (strcmp(label, "LOAD") != 0 && strcmp(label, "LOADI") != 0 &&
          strcmp(label, "STORE") != 0 && strcmp(label, "STOREI") != 0 &&
          strcmp(label, "JMP") != 0 && strcmp(label, "JMPZ") != 0 &&
          strcmp(label, "JMPN") != 0 && strcmp(label, "JMPO") != 0 &&
          strcmp(label, "ADD") != 0 && strcmp(label, "ADDR") != 0 &&
          strcmp(label, "SUB") != 0 && strcmp(label, "SUBR") != 0 &&
          strcmp(label, "OUT") != 0 && strcmp(label, "OUTC") != 0 &&
          strcmp(label, "OUTR") != 0 && strcmp(label, "OUTRC") != 0 &&
          strcmp(label, "OUTI") != 0 && strcmp(label, "OUTIC") != 0 &&
          strcmp(label, "HALT") != 0 && strcmp(label, "DATA") != 0) {

        add_label(label, location_counter);
        strcpy(line_copy, rest_of_line); // Remove label from line
      }

      // Record the address of this instruction
      instruction_addresses[i] = location_counter;

      // Determine instruction size
      char instruction[MAX_LINE_LENGTH];
      sscanf(line_copy, " %s", instruction);

      if (strcmp(instruction, "HALT") == 0) {
        location_counter += 1; // HALT occupies 1 byte
      } else if (strcmp(instruction, "LOAD") == 0 ||
                 strcmp(instruction, "STORE") == 0 ||
                 strcmp(instruction, "JMP") == 0 ||
                 strcmp(instruction, "JMPZ") == 0 ||
                 strcmp(instruction, "JMPN") == 0 ||
                 strcmp(instruction, "JMPO") == 0 ||
                 strcmp(instruction, "ADD") == 0 ||
                 strcmp(instruction, "SUB") == 0 ||
                 strcmp(instruction, "OUT") == 0 ||
                 strcmp(instruction, "OUTC") == 0) {
        location_counter += 4; // These instructions occupy 4 bytes
      } else if (strcmp(instruction, "LOADI") == 0 ||
                 strcmp(instruction, "STOREI") == 0 ||
                 strcmp(instruction, "ADDR") == 0 ||
                 strcmp(instruction, "SUBR") == 0 ||
                 strcmp(instruction, "OUTR") == 0 ||
                 strcmp(instruction, "OUTRC") == 0 ||
                 strcmp(instruction, "OUTI") == 0 ||
                 strcmp(instruction, "OUTIC") == 0) {
        location_counter += 2; // These instructions occupy 2 bytes
      } else if (strcmp(instruction, "DATA") == 0) {
        location_counter += 2; // DATA occupies 2 bytes
      } else {
        fprintf(stderr, "Unknown instruction in first pass: %s\n", instruction);
        exit(1);
      }

      // Update lines[i] for the second pass
      strcpy(lines[i], line_copy);

    } else {
      // No label on this line
      instruction_addresses[i] = location_counter;

      // Determine instruction size
      char instruction[MAX_LINE_LENGTH];
      sscanf(line_copy, " %s", instruction);

      if (strcmp(instruction, "DATA") == 0) {
        location_counter += 2; // DATA occupies 2 bytes
      } else if (strcmp(instruction, "HALT") == 0) {
        location_counter += 1; // HALT occupies 1 byte
      } else if (strcmp(instruction, "LOAD") == 0 ||
                 strcmp(instruction, "STORE") == 0 ||
                 strcmp(instruction, "JMP") == 0 ||
                 strcmp(instruction, "JMPZ") == 0 ||
                 strcmp(instruction, "JMPN") == 0 ||
                 strcmp(instruction, "JMPO") == 0 ||
                 strcmp(instruction, "ADD") == 0 ||
                 strcmp(instruction, "SUB") == 0 ||
                 strcmp(instruction, "OUT") == 0 ||
                 strcmp(instruction, "OUTC") == 0) {
        location_counter += 4; // These instructions occupy 4 bytes
      } else if (strcmp(instruction, "LOADI") == 0 ||
                 strcmp(instruction, "STOREI") == 0 ||
                 strcmp(instruction, "ADDR") == 0 ||
                 strcmp(instruction, "SUBR") == 0 ||
                 strcmp(instruction, "OUTR") == 0 ||
                 strcmp(instruction, "OUTRC") == 0 ||
                 strcmp(instruction, "OUTI") == 0 ||
                 strcmp(instruction, "OUTIC") == 0) {
        location_counter += 2; // These instructions occupy 2 bytes
      } else {
        fprintf(stderr, "Unknown instruction in first pass: %s\n", instruction);
        exit(1);
      }
    }
  }
}

/**
 * Second pass of the assembler: generates machine code.
 *
 * @param lines The array of code lines.
 * @param line_count The number of lines.
 * @param instruction_addresses Array of instruction addresses.
 */
void second_pass(char lines[][MAX_LINE_LENGTH], int line_count) {
  for (int i = 0; i < line_count; i++) {
    char line_copy[MAX_LINE_LENGTH];
    strcpy(line_copy, lines[i]);
    strip_comments(line_copy);
    trim_whitespace(line_copy);

    if (strlen(line_copy) == 0)
      continue;

    char instruction[MAX_LINE_LENGTH];
    char operand1[MAX_LINE_LENGTH];
    char operand2[MAX_LINE_LENGTH];

    // Parse instruction line
    if (sscanf(line_copy, " %s %[^,], %s", instruction, operand1, operand2) ==
        3) {
      // Instructions with two operands
      if (strcmp(instruction, "LOAD") == 0 || strcmp(instruction, "ADD") == 0 ||
          strcmp(instruction, "SUB") == 0 ||
          strcmp(instruction, "STORE") == 0) {

        uint8_t opcode = 0;
        if (strcmp(instruction, "LOAD") == 0)
          opcode = LOAD;
        else if (strcmp(instruction, "ADD") == 0)
          opcode = ADD;
        else if (strcmp(instruction, "SUB") == 0)
          opcode = SUB;
        else if (strcmp(instruction, "STORE") == 0)
          opcode = STORE;

        uint8_t reg_code = get_register_code(operand1);
        if (reg_code == 0xFF) {
          fprintf(stderr, "Invalid register: %s\n", operand1);
          exit(1);
        }

        uint16_t immediate;
        if (find_label(operand2, &immediate) == 0) {
          immediate = (uint16_t)atoi(operand2);
        }

        putchar(opcode);
        putchar(reg_code);
        write16(immediate);

      } else if (strcmp(instruction, "LOADI") == 0 ||
                 strcmp(instruction, "STOREI") == 0 ||
                 strcmp(instruction, "ADDR") == 0 ||
                 strcmp(instruction, "SUBR") == 0) {

        uint8_t opcode = 0;
        if (strcmp(instruction, "LOADI") == 0)
          opcode = LOADI;
        else if (strcmp(instruction, "STOREI") == 0)
          opcode = STOREI;
        else if (strcmp(instruction, "ADDR") == 0)
          opcode = ADDR;
        else if (strcmp(instruction, "SUBR") == 0)
          opcode = SUBR;

        uint8_t reg_code1 =
            get_register_code(operand1); // Destination register (reg1)
        uint8_t reg_code2 =
            get_register_code(operand2); // Source or address register (reg2)

        if (reg_code1 == 0xFF || reg_code2 == 0xFF) {
          fprintf(stderr, "Invalid register in instruction: %s %s, %s\n",
                  instruction, operand1, operand2);
          exit(1);
        }

        uint8_t reg_byte = (reg_code2 << 6) | (reg_code1 & 0x03);
        putchar(opcode);
        putchar(reg_byte);

      } else {
        fprintf(stderr, "Unknown instruction with two operands: %s\n",
                instruction);
        exit(1);
      }

    } else if (sscanf(line_copy, " %s %s", instruction, operand1) == 2) {
      // Instructions with one operand
      if (strcmp(instruction, "JMP") == 0 || strcmp(instruction, "JMPZ") == 0 ||
          strcmp(instruction, "JMPN") == 0 ||
          strcmp(instruction, "JMPO") == 0 ||
          strcmp(instruction, "DATA") == 0 ||
          strcmp(instruction, "OUTR") == 0 ||
          strcmp(instruction, "OUTRC") == 0 ||
          strcmp(instruction, "OUTI") == 0 ||
          strcmp(instruction, "OUTIC") == 0 ||
          strcmp(instruction, "OUT") == 0 || strcmp(instruction, "OUTC") == 0) {

        if (strcmp(instruction, "OUTR") == 0 ||
            strcmp(instruction, "OUTRC") == 0 ||
            strcmp(instruction, "OUTI") == 0 ||
            strcmp(instruction, "OUTIC") == 0) {

          uint8_t opcode = 0;
          if (strcmp(instruction, "OUTR") == 0)
            opcode = OUTR;
          else if (strcmp(instruction, "OUTRC") == 0)
            opcode = OUTRC;
          else if (strcmp(instruction, "OUTI") == 0)
            opcode = OUTI;
          else if (strcmp(instruction, "OUTIC") == 0)
            opcode = OUTIC;

          uint8_t reg_code = get_register_code(operand1);
          if (reg_code == 0xFF) {
            fprintf(stderr, "Invalid register: %s\n", operand1);
            exit(1);
          }

          putchar(opcode);
          putchar(reg_code);

        } else if (strcmp(instruction, "OUT") == 0 ||
                   strcmp(instruction, "OUTC") == 0) {
          uint8_t opcode = (strcmp(instruction, "OUT") == 0) ? OUT : OUTC;

          uint16_t immediate;
          if (find_label(operand1, &immediate) == 0) {
            immediate = (uint16_t)atoi(operand1);
          }

          putchar(opcode);
          putchar(0); // Unused byte
          write16(immediate);

        } else if (strcmp(instruction, "DATA") == 0) {
          uint16_t value;
          if (find_label(operand1, &value) == 0) {
            value = (uint16_t)atoi(operand1);
          }
          write16(value);

        } else {
          // Handle JMP and its variants
          uint8_t opcode = 0;
          if (strcmp(instruction, "JMP") == 0)
            opcode = JMP;
          else if (strcmp(instruction, "JMPZ") == 0)
            opcode = JMPZ;
          else if (strcmp(instruction, "JMPN") == 0)
            opcode = JMPN;
          else if (strcmp(instruction, "JMPO") == 0)
            opcode = JMPO;

          uint16_t address;
          if (find_label(operand1, &address) == 0) {
            fprintf(stderr, "Error: Undefined label %s\n", operand1);
            exit(1);
          }

          putchar(opcode);
          putchar(0); // Unused byte
          write16(address);
        }

      } else {
        fprintf(stderr, "Unknown instruction with one operand: %s\n",
                instruction);
        exit(1);
      }
    } else if (sscanf(line_copy, " %s", instruction) == 1) {
      // Instructions with no operands
      if (strcmp(instruction, "HALT") == 0) {
        putchar(HALT);
      } else {
        fprintf(stderr, "Unknown instruction: %s\n", instruction);
        exit(1);
      }
    } else {
      fprintf(stderr, "Invalid instruction format: %s\n", line_copy);
      exit(1);
    }
  }
}

/**
 * Main function of the assembler.
 *
 * @return Exit status code.
 */
int main() {
  char lines[1024][MAX_LINE_LENGTH];
  uint16_t instruction_addresses[1024];
  int line_count = 0;

  // Read all lines from stdin
  while (fgets(lines[line_count], sizeof(lines[line_count]), stdin)) {
    // Remove any trailing newlines
    lines[line_count][strcspn(lines[line_count], "\n")] = 0;
    line_count++;
  }

  // First pass: build symbol table
  first_pass(lines, line_count, instruction_addresses);

  // Second pass: generate machine code
  second_pass(lines, line_count);

  return 0;
}
