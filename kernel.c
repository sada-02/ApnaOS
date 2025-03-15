void kernel_main() {
    char *video_memory = (char*) 0xb8000;
    video_memory[0] = 'X';  // Display 'X' on screen
    video_memory[1] = 0x07; // White-on-black color
}


