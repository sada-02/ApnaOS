# ApnaOS
command to run on wsl: `make run`
dependencies needed: grub wagerah, xorriso, cross compiler (i686 gcc something)

# 1. Assemble the bootloader
nasm -f elf32 boot.asm -o boot.o

# 2. Compile the C files (using -m32 and -ffreestanding)
gcc -m32 -ffreestanding -c kernel.c -o kernel.o
gcc -m32 -ffreestanding -c memory.c -o memory.o
gcc -m32 -ffreestanding -c process.c -o process.o
gcc -m32 -ffreestanding -c process_test.c -o process_test.o
gcc -m32 -ffreestanding -c test_processes/dummy1.c -o dummy1.o
gcc -m32 -ffreestanding -c test_processes/dummy2.c -o dummy2.o
gcc -m32 -ffreestanding -c test_processes/dummy3.c -o dummy3.o

# (If using a separate debug.c, compile it too)
# gcc -m32 -ffreestanding -c debug.c -o debug.o

# 3. Link all object files into a kernel binary.
# If you compiled debug.o, include it in the list.
ld -m elf_i386 -T linker.ld -o kernel.bin boot.o kernel.o memory.o process.o process_test.o dummy1.o dummy2.o dummy3.o

# 4. Set up the GRUB boot structure.
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/
cp grub.cfg iso/boot/grub/

# 5. Create a bootable ISO image using grub-mkrescue.
grub-mkrescue -o os.iso iso

# 6. Run the ISO in QEMU.
qemu-system-i386 -cdrom os.iso

code to run this all
