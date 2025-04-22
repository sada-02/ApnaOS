#include "vmm.h"
#include "vmx.h"

int vmm_init() {
    if (!check_vmx_support()) return 0;
    enable_vmx();
    return vmxon();
}
