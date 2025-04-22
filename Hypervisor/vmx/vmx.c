#include "vmx.h"
#include <stdint.h>

// Make sure this is available from memory.c
extern void* allocate_pages(1);  // Allocates 4KB aligned page

static uint8_t* vmxon_region = 0;

// Read MSR
static inline uint64_t read_msr(uint32_t msr) {
    uint32_t low, high;
    __asm__ volatile ("rdmsr" : "=a"(low), "=d"(high) : "c"(msr));
    return ((uint64_t)high << 32) | low;
}

int check_vmx_support() {
    uint32_t eax, ebx, ecx, edx;
    __asm__ volatile("cpuid"
                     : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)
                     : "a"(1));
    return (ecx & (1 << 5)) != 0;  // ECX bit 5: VMX available
}

void enable_vmx() {
    uint32_t cr4;
    __asm__ volatile("mov %%cr4, %0" : "=r"(cr4));
    cr4 |= (1 << 13);  // Set CR4.VMXE
    __asm__ volatile("mov %0, %%cr4" :: "r"(cr4));
}

int vmxon() {
    if (!vmxon_region) {
        vmxon_region = (uint8_t*)allocate_pages(1);
        if (!vmxon_region) return 0;
    }

    uint64_t basic_info = read_msr(0x480); // IA32_VMX_BASIC
    uint32_t rev_id = (uint32_t)(basic_info & 0xFFFFFFFF);
    *((uint32_t*)vmxon_region) = rev_id;

    uint32_t vmxon_phys = (uint32_t)vmxon_region;
    int status;
    __asm__ volatile("vmxon %1; setna %0"
                     : "=r"(status)
                     : "m"(vmxon_phys)
                     : "memory");
    return status == 0;
}

int vmxoff() {
    int result;
    __asm__ volatile("vmxoff; setna %0" : "=r"(result));
    return result == 0;
}

int vmclear(uint32_t* phys_addr) {
    int status;
    __asm__ volatile("vmclear %1; setna %0"
                     : "=r"(status)
                     : "m"(*phys_addr));
    return status == 0;
}

int vmptrld(uint32_t* phys_addr) {
    int status;
    __asm__ volatile("vmptrld %1; setna %0"
                     : "=r"(status)
                     : "m"(*phys_addr));
    return status == 0;
}

int vmlaunch() {
    int result;
    __asm__ volatile("vmlaunch; setna %0" : "=r"(result) :: "memory");
    return result == 0;
}

int vmwrite(uint32_t field, uint32_t value) {
    int result;
    __asm__ volatile("vmwrite %1, %2; setna %0"
                     : "=r"(result)
                     : "r"(value), "r"(field)
                     : "memory");
    return result == 0;
}

int vmread(uint32_t field, uint32_t* value_out) {
    int result;
    __asm__ volatile("vmread %2, %1; setna %0"
                     : "=r"(result), "=r"(*value_out)
                     : "r"(field)
                     : "memory");
    return result == 0;
}
