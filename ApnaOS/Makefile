# Compiler and Assembler flags
CC = gcc
ASM = nasm
LD = ld
CFLAGS = -m32 -ffreestanding -c
LDFLAGS = -m elf_i386 -T linker.ld
ASFLAGS = -f elf32

# Files
SRC_C = kernel.c memory.c process.c process_test.c filesystem.c test_processes/dummy1.c test_processes/dummy2.c test_processes/dummy3.c
SRC_ASM = boot.asm
OBJ = boot.o kernel.o memory.o process.o process_test.o filesystem.o dummy1.o dummy2.o dummy3.o
KERNEL = kernel.bin
ISO_DIR = iso/boot
ISO = os.iso

# Default target: build everything
all: $(ISO)

# Assemble bootloader
boot.o: boot.asm
	$(ASM) $(ASFLAGS) boot.asm -o boot.o

# Compile kernel
kernel.o: kernel.c
	$(CC) $(CFLAGS) kernel.c -o kernel.o

# Compile memory management
memory.o: memory.c
	$(CC) $(CFLAGS) memory.c -o memory.o

# Compile process management
process.o: process.c
	$(CC) $(CFLAGS) process.c -o process.o

# Compile process test
process_test.o: process_test.c
	$(CC) $(CFLAGS) process_test.c -o process_test.o

# Compile filesystem
filesystem.o: filesystem.c
	$(CC) $(CFLAGS) filesystem.c -o filesystem.o

# Compile dummy processes
dummy1.o: test_processes/dummy1.c
	$(CC) $(CFLAGS) test_processes/dummy1.c -o dummy1.o

dummy2.o: test_processes/dummy2.c
	$(CC) $(CFLAGS) test_processes/dummy2.c -o dummy2.o

dummy3.o: test_processes/dummy3.c
	$(CC) $(CFLAGS) test_processes/dummy3.c -o dummy3.o

# Link everything
$(KERNEL): $(OBJ) linker.ld
	$(LD) $(LDFLAGS) -o $(KERNEL) $(OBJ)

# Prepare ISO directory
prepare-iso: $(KERNEL)
	mkdir -p $(ISO_DIR)/grub
	cp $(KERNEL) $(ISO_DIR)/

# Create GRUB config
iso/boot/grub/grub.cfg:
	echo 'set timeout=0' > iso/boot/grub/grub.cfg
	echo 'set default=0' >> iso/boot/grub/grub.cfg
	echo 'menuentry "MyOS" {' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg

# Build ISO
$(ISO): prepare-iso iso/boot/grub/grub.cfg
	grub-mkrescue -o $(ISO) iso/ --modules="multiboot"

# Run in QEMU
run: $(ISO)
	qemu-system-i386 -cdrom $(ISO)

# Clean build files
clean:
	rm -rf $(OBJ) $(KERNEL) $(ISO) iso/