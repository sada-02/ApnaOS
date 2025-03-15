# Compiler and Assembler flags
CC = gcc
ASM = nasm
LD = ld
CFLAGS = -m32 -ffreestanding -c
LDFLAGS = -m elf_i386 -T linker.ld
ASFLAGS = -f elf32

# Files
SRC_C = kernel.c
SRC_ASM = boot.asm
OBJ = boot.o kernel.o
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
	echo '    multiboot /boot/kernel.bin' >> iso/boot/grub/grub.cfg
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
