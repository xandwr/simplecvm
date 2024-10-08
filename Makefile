# Makefile for the Simple Virtual Machine and Assembler
# Author: Xander Pickering (3118504)
# Updated: 2024/10/07


# Compiler to use
CC = gcc

# Compiler flags for warnings and standards compliance
CFLAGS = -Wall -Wextra -std=c11

# Auto-clean (convenience)
AUTOCLEAN = 1

# Executable names
EXECUTABLES = sasm svm

# Test files
TESTS = test1 test2 factors

# Declare phony targets to prevent conflicts
.PHONY: all clean test

# Default target that builds executables and runs tests
all: sasm svm test

# Rule to build the assembler
sasm: sasm.c svm.h
	@echo "\n##########"
	@echo "Makefile for the Simple Virtual Machine and Assembler"
	@echo "Author: Xander Pickering (3118504)"
	@echo "Updated: 2024/10/07"
	@echo "##########"
	@echo "\n\n## 1. COMPILATION ##"
	@echo "\nCompiling sasm..."
	$(CC) $(CFLAGS) -o sasm sasm.c
	@echo "...sasm compile successful!"

# Rule to build the virtual machine
svm: svm.c svm.h
	@echo "\nCompiling svm..."
	$(CC) $(CFLAGS) -o svm svm.c
	@echo "...svm compile successful!"

# Rule to run tests
test: 
	@echo "\n\n## 2. TESTING ##"
	@$(MAKE) $(TESTS:%=test_%)
	@echo "\nAll tests completed. Goodbye!"
	@if [ "$(AUTOCLEAN)" -eq "1" ]; then \
		$(MAKE) clean; \
	fi

# Pattern rule to assemble and run each test
test_%: sasm svm tests/%.svm
	@echo "\nAssembling and running test '$*'..."
	@mkdir -p tests/bin
	@echo "\nAssembling '$*.svm' into binary..."
	./sasm < tests/$*.svm > tests/bin/$*.bin
	@echo "\nRunning '$*.bin' with svm..."
	./svm < tests/bin/$*.bin > tests/$*.output
	@if [ -f tests/$*.expected ]; then \
		echo "\nComparing output for test '$*'..."; \
		if diff -q tests/$*.output tests/$*.expected >/dev/null; then \
			echo "Test '$*' passed!"; \
		else \
			echo "Test '$*' failed. Output differs from expected."; \
		fi \
	else \
		echo "\nTest '$*' output:"; \
		cat tests/$*.output; \
	fi
	@echo

# Clean up generated files
clean:
	@echo "\n\n## 3. CLEANUP ##"
	@echo "\nCleaning up all generated files..."
	rm -f $(EXECUTABLES)
	rm -f tests/*.output
	rm -rf tests/bin
	@echo "\nAll generated files have been removed. Goodbye!"
