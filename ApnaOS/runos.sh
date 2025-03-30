#!/bin/bash
set -e

echo "Assembling bootloader..."
nasm -f elf32 boot.asm -o boot.o
nasm -f elf32 idt.asm -o idt_asm.o
nasm -f elf32 exceptions.asm -o exceptions.o
nasm -f elf32 irq.asm -o irq_asm.o
nasm -f elf32 gdt.asm -o gdt.o

echo "Compiling C files..."
gcc -m32 -ffreestanding -c kernel.c -o kernel.o
gcc -m32 -ffreestanding -c gdt.c -o gdt_c.o
gcc -m32 -ffreestanding -c memory.c -o memory.o
gcc -m32 -ffreestanding -c filesystem.c -o filesystem.o
gcc -m32 -ffreestanding -c serial.c -o serial.o
gcc -m32 -ffreestanding -c process.c -o process.o
gcc -m32 -ffreestanding -c process_test.c -o process_test.o
gcc -m32 -ffreestanding -c keyboard.c -o keyboard.o
gcc -m32 -ffreestanding -c syscall.c -o syscall.o
gcc -m32 -ffreestanding -c syscall_test.c -o syscall_test.o
gcc -m32 -ffreestanding -c io.c -o io.o
gcc -m32 -ffreestanding -c idt.c -o idt.o
gcc -m32 -ffreestanding -c pic.c -o pic.o
gcc -m32 -ffreestanding -c interrupts.c -o interrupts.o
gcc -m32 -ffreestanding -c string.c -o string.o
gcc -m32 -ffreestanding -c test_processes/dummy1.c -o dummy1.o
gcc -m32 -ffreestanding -c test_processes/dummy2.c -o dummy2.o
gcc -m32 -ffreestanding -c test_processes/dummy3.c -o dummy3.o

echo "Linking kernel binary..."
ld -m elf_i386 -T linker.ld -o kernel.bin \
    boot.o kernel.o process.o process_test.o serial.o keyboard.o memory.o \
    io.o idt.o idt_asm.o irq_asm.o pic.o interrupts.o gdt.o gdt_c.o exceptions.o \
    string.o dummy1.o dummy2.o dummy3.o filesystem.o syscall.o syscall_test.o

echo "Setting up GRUB boot structure..."
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/
cp grub.cfg iso/boot/grub/

echo "Creating bootable ISO image..."
grub-mkrescue -o os.iso iso

echo "Running OS in QEMU..."
qemu-system-i386 \
    -kernel kernel.bin \
    -cdrom os.iso \
    -serial stdio \
    -vga std

echo "Done."
