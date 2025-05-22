# ApnaOS

**ApnaOS** is a minimalistic, educational operating system kernel written in C and x86 Assembly. It demonstrates core OS concepts including bootloading, interrupt handling, memory management, process scheduling, file management, and basic I/O drivers. The kernel is Multiboot-compliant and can be built and run in QEMU for rapid development and testing.

## Table of Contents

* [Project Layout](#project-layout)
* [Prerequisites](#prerequisites)
* [Installation](#installation)
* [Building and Running](#building-and-running)
* [Automated Script](#automated-script)
* [Modules & Directories](#modules--directories)
* [Contributing](#contributing)
* [License](#license)

## Project Layout

```
ApnaOS/                  # Repository root
├── .vscode/             # Editor settings (optional)
├── bin/                 # Prebuilt binaries (if any)
├── filesystem/          # File management code
├── interrupts/          # Interrupt Service Routines
├── iso/                 # ISO staging directory
│   └── boot/
├── keyboard/            # Keyboard driver
├── memory/              # Memory management (paging, heap, etc.)
├── process/             # Process management and scheduling
├── test_processes/      # Sample user programs
├── Makefile             # Top-level build targets
├── runos.sh             # Convenience script: build + run
├── boot.asm             # Multiboot bootloader
├── grub.cfg             # GRUB configuration (Multiboot entry)
├── linker.ld            # Linker script for a freestanding 32-bit kernel
├── kernel.c             # Entry point and kernel initialization
├── serial.c             # Serial port driver
├── serial.h             # Serial interface headers
├── kernel.bin           # Linked kernel binary (build artifact)
├── kernel.elf           # ELF image (build artifact)
├── apnaos.iso           # ISO image (build artifact)
└── os.iso               # Alternate ISO build
```

## Prerequisites

You will need the following tools installed on your development machine:

* **GNU Make**
* **NASM** (Netwide Assembler)
* **GCC** with 32-bit support (`-m32`, `-ffreestanding`)
* **GNU Binutils** (`ld` for linking)
* **GRUB 2** (`grub-mkrescue`)
* **xorriso** (for hybrid ISO creation)
* **QEMU** (`qemu-system-i386`)
* Standard Unix utilities: `mkdir`, `cp`, `rm`, etc.

On **Ubuntu/Debian**:

```bash
sudo apt update
sudo apt install \
  build-essential \
  gcc-multilib \
  nasm \
  binutils \
  grub-pc-bin \
  xorriso \
  qemu-system-i386 \
  make
```

On **Arch Linux**:

```bash
sudo pacman -S \
  base-devel \
  gcc-multilib \
  nasm \
  binutils \
  grub \
  xorriso \
  qemu \
  make
```

## Installation

1. **Clone the repository**:

   ```bash
   git clone https://github.com/sada-02/ApnaOS.git
   cd ApnaOS
   ```
2. **Verify tools**:

   ```bash
   make --version && \
   nasm -v && \
   gcc -m32 -v && \
   grub-mkrescue --version && \
   xorriso --version && \
   qemu-system-i386 --version
   ```

## Building and Running

Use the Makefile to build and run the OS:

```bash
# build bootloader, kernel, ISO
make all

# run in QEMU
make run
```

Under the hood, the Makefile executes the following steps:

1. Assemble `boot.asm` → `boot.o`
2. Compile core modules (`kernel.c`, drivers, memory, process, interrupts, filesystem) with `gcc -m32 -ffreestanding -c`
3. Compile test programs under `test_processes/`
4. Link all object files via `ld -m elf_i386 -T linker.ld` → `kernel.bin` & `kernel.elf`
5. Set up GRUB staging in `iso/boot/`, copying `kernel.bin` & `grub.cfg`
6. Create a bootable ISO (`apnaos.iso` / `os.iso`) using `grub-mkrescue` & `xorriso`
7. Launch with `qemu-system-i386 -cdrom apnaos.iso`

All these steps are mirrored in `runos.sh` for convenience.

## Automated Script

Alternatively, you can execute the bundled shell script:

```bash
./runos.sh
```

This will clean previous builds, rebuild the kernel, generate the ISO, and launch QEMU.

## Modules & Directories

* **boot.asm**: Multiboot-compliant assembly stub and real-mode setup.
* **kernel.c**: Kernel entry, GDT/IDT setup, C-level init, main loop.
* **serial.c/h**: Serial port initialization & I/O routines.
* **interrupts/**: ISR definitions, IDT installation.
* **memory/**: Paging, heap allocator, memory map parsing.
* **keyboard/**: PS/2 keyboard driver, keycode processing.
* **process/**: Process Control Block, scheduler (e.g., round-robin), context switching.
* **filesystem/**: Simple file abstraction layer (in-memory/file descriptor handling).
* **test\_processes/**: Sample user-space binaries demonstrating process scheduling and syscalls.
* **iso/boot/grub/**: GRUB config and kernel staging for bootloader.
* **Makefile**: Build targets (`all`, `clean`, `run`, etc.) and dependency rules.

## Contributing

Contributions are welcome! To contribute:

1. Fork the repository
2. Create a feature branch (`git checkout -b feature/xyz`)
3. Commit your changes, following existing code style
4. Submit a Pull Request for review

Please ensure any new code is documented and accompanied by tests where applicable.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for full details.
