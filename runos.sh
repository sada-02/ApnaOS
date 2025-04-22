#!/bin/bash
set -e

echo "Assembling bootloader..."
nasm -f elf32 boot.asm -o bin/boot.o
nasm -f elf32 interrupts/idt.asm -o bin/idt_asm.o
nasm -f elf32 interrupts/exceptions.asm -o bin/exceptions.o
nasm -f elf32 interrupts/irq.asm -o bin/irq_asm.o
nasm -f elf32 keyboard/gdt.asm -o bin/gdt.o

echo "Compiling C files..."
gcc -m32 -ffreestanding -c kernel.c -o bin/kernel.o
gcc -m32 -ffreestanding -c serial.c -o bin/serial.o

gcc -m32 -ffreestanding -c memory/memory.c -o bin/memory.o
gcc -m32 -ffreestanding -c filesystem/filesystem.c -o bin/filesystem.o

gcc -m32 -ffreestanding -c process/process.c -o bin/process.o
gcc -m32 -ffreestanding -c process/syscall.c -o bin/syscall.o

gcc -m32 -ffreestanding -c keyboard/keyboard.c -o bin/keyboard.o
gcc -m32 -ffreestanding -c keyboard/io.c -o bin/io.o
gcc -m32 -ffreestanding -c keyboard/string.c -o bin/string.o
gcc -m32 -ffreestanding -c keyboard/gdt.c -o bin/gdt_c.o

gcc -m32 -ffreestanding -c interrupts/idt.c -o bin/idt.o
gcc -m32 -ffreestanding -c interrupts/pic.c -o bin/pic.o
gcc -m32 -ffreestanding -c interrupts/interrupts.c -o bin/interrupts.o

gcc -m32 -ffreestanding -c test_processes/dummy1.c -o bin/dummy1.o
gcc -m32 -ffreestanding -c test_processes/dummy2.c -o bin/dummy2.o
gcc -m32 -ffreestanding -c test_processes/dummy3.c -o bin/dummy3.o
gcc -m32 -ffreestanding -c test_processes/process_test.c -o bin/process_test.o
gcc -m32 -ffreestanding -c test_processes/syscall_test.c -o bin/syscall_test.o

echo "Linking kernel binary..."
ld -m elf_i386 -T linker.ld -o kernel.bin \
    bin/boot.o bin/kernel.o bin/process.o bin/process_test.o bin/serial.o bin/keyboard.o bin/memory.o \
    bin/io.o bin/idt.o bin/idt_asm.o bin/irq_asm.o bin/pic.o bin/interrupts.o bin/gdt.o bin/gdt_c.o bin/exceptions.o \
    bin/string.o bin/dummy1.o bin/dummy2.o bin/dummy3.o bin/filesystem.o bin/syscall.o bin/syscall_test.o

echo "Setting up GRUB boot structure..."
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/
cp grub.cfg iso/boot/grub/

echo "Creating bootable ISO image..."
grub-mkrescue -o os.iso iso

echo "Running OS in QEMU..."
qemu-system-i386 \
  -cdrom os.iso \
  -boot d \
  -serial stdio \
  -vga std

echo "Done."
