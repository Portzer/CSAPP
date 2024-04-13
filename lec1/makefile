CC = /usr/bin/gcc
CFLAGS = -Wall -g -O2 -Werror -std=gnu99 -Wno-unused-function

EXE_HARDWARE = exe_hardware

SRC_DIR = ./src

# debug
COMMON = $(SRC_DIR)/common/print.c $(SRC_DIR)/common/convert.c

# hardware
CPU =$(SRC_DIR)/hardware/cpu/mmu.c $(SRC_DIR)/hardware/cpu/isa.c
MEMORY =$(SRC_DIR)/hardware/memory/dram.c
# main
MAIN_HARDWARE = $(SRC_DIR)/main.c

.PHONY:hardware
hardware:
	$(CC) $(CFLAGS) -I$(SRC_DIR) $(COMMON) $(CPU) $(MEMORY)  $(MAIN_HARDWARE) -o $(EXE_HARDWARE)
	./$(EXE_HARDWARE)
.PHONY:link
link:
	$(CC) $(CFLAGS) -I$(SRC_DIR) $(COMMON) $(SRC_DIR)/test/test_elf.c $(SRC_DIR)/linker/parseElf.c  $(SRC_DIR)/linker/static_link.c -o ./bin/test_elf
	./bin/test_elf
clean:
	rm -f *.o *~ $(EXE_HARDWARE)