#ifndef VM_DETECTOR_H
#define VM_DETECTOR_H

// Include necessary headers
#include <string>

// Function declarations
bool DetectVirtualBox();
bool DetectVMware();
bool DetectHypervisor();
bool DetectProcessorBrand();
bool DetectSandbox();
bool IsRunningInVM();

#endif // VM_DETECTOR_H
