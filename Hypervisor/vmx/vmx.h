#ifndef VMX_H
#define VMX_H

// Check if CPU supports VT-x
int check_vmx_support();

// Enable VMX via CR4
void enable_vmx();

// Enter and exit VMX root operation
int vmxon();
int vmxoff();

// VMCS operations
int vmclear(uint32_t* phys_addr);
int vmptrld(uint32_t* phys_addr);
int vmlaunch();
int vmwrite(uint32_t field, uint32_t value);
int vmread(uint32_t field, uint32_t* value_out);

#endif
