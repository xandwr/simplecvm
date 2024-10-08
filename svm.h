#ifndef SVM_H
#define SVM_H

// Memory size (32kb)
#define MEMORY_SIZE 32768

// Maximum line length for reading assembly code
#define MAX_LINE_LENGTH 100

// Opcode definitions
#define HALT 0x31
#define LOAD 0x60
#define LOADI 0x61
#define STORE 0x62
#define STOREI 0x63
#define JMP 0x64
#define JMPZ 0x65
#define JMPN 0x66
#define JMPO 0x67
#define ADD 0x68
#define ADDR 0x69
#define SUB 0x6a
#define SUBR 0x6b
#define OUT 0x6c
#define OUTC 0x6d
#define OUTR 0x6e
#define OUTRC 0x6f
#define OUTI 0x70
#define OUTIC 0x71

// Register definitions
#define A1 3
#define A2 2
#define R1 1
#define R2 0

#endif // SVM_H
