#!/bin/bash
set -e

echo "Assembling bootloader..."
nasm -f elf32 boot.asm -o boot.o

echo "Compiling C files..."
gcc -m32 -ffreestanding -c kernel.c -o kernel.o
gcc -m32 -ffreestanding -c memory.c -o memory.o
gcc -m32 -ffreestanding -c process.c -o process.o
gcc -m32 -ffreestanding -c process_test.c -o process_test.o
gcc -m32 -ffreestanding -c serial.c -o serial.o
# gcc -m32 -ffreestanding -c test_processes/dummy1.c -o dummy1.o
# gcc -m32 -ffreestanding -c test_processes/dummy2.c -o dummy2.o
# gcc -m32 -ffreestanding -c test_processes/dummy3.c -o dummy3.o
# gcc -m32 -ffreestanding -c test_processes/fork_exec_test.c -o fork_exec_test.o
# gcc -m32 -ffreestanding -c context_switch_handler.c -o context_switch_handler.o
gcc -m32 -ffreestanding -c keyboard.c -o keyboard.o

# echo "Assembling advanced context switch routine..."
# gcc -m32 -ffreestanding -c timer_int_handler.s -o timer_int_handler.o
# gcc -m32 -ffreestanding -c switch_context.s -o switch_context.o

echo "Linking kernel binary..."
gcc -m32 -ffreestanding -nostdlib -T linker.ld -o kernel.bin \
    boot.o kernel.o process.o process_test.o \
    serial.o keyboard.o memory.o\
    #memory.o context_switch_handler.o timer_int_handler.o switch_context.o
    
echo "Setting up GRUB boot structure..."
mkdir -p iso/boot/grub
cp kernel.bin iso/boot/
cp grub.cfg iso/boot/grub/

echo "Creating bootable ISO image..."
grub-mkrescue -o os.iso iso

echo "Running OS in QEMU..."
qemu-system-i386 -cdrom os.iso -serial stdio

echo "Done."
