#include "vm_detector.h"
#include <fstream>
#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/utsname.h>

bool DetectVirtualBox() {
    std::ifstream dmiInfo("/sys/class/dmi/id/product_name");
    if (dmiInfo.is_open()) {
        std::string productName;
        std::getline(dmiInfo, productName);
        dmiInfo.close();
        return productName.find("VirtualBox") != std::string::npos;
    }
    return false;
}

bool DetectVMware() {
    std::ifstream dmiInfo("/sys/class/dmi/id/product_name");
    if (dmiInfo.is_open()) {
        std::string productName;
        std::getline(dmiInfo, productName);
        dmiInfo.close();
        return productName.find("VMware") != std::string::npos;
    }
    return false;
}

bool DetectHypervisor() {
    std::ifstream cpuInfo("/proc/cpuinfo");
    if (cpuInfo.is_open()) {
        std::string line;
        while (std::getline(cpuInfo, line)) {
            if (line.find("hypervisor") != std::string::npos) {
                cpuInfo.close();
                return true;
            }
        }
        cpuInfo.close();
    }
    return false;
}

bool DetectProcessorBrand() {
    std::ifstream cpuInfo("/proc/cpuinfo");
    if (cpuInfo.is_open()) {
        std::string line;
        while (std::getline(cpuInfo, line)) {
            if (line.find("model name") != std::string::npos) {
                if (line.find("Virtual") != std::string::npos || line.find("VMware") != std::string::npos) {
                    cpuInfo.close();
                    return true;
                }
            }
        }
        cpuInfo.close();
    }
    return false;
}

bool DetectSandbox() {
    struct utsname unameData;
    if (uname(&unameData) == 0) {
        std::string sysname(unameData.sysname);
        return sysname.find("Linuxkit") != std::string::npos;
    }
    return false;
}

bool IsRunningInVM() {
    typedef bool (*DetectionFunction)();

    DetectionFunction detections[] = {
        DetectVirtualBox,
        DetectVMware,
        DetectHypervisor,
        DetectProcessorBrand,
        DetectSandbox
    };

    for (auto& detect : detections) {
        if (detect()) {
            return true;
        }
    }
    return false;
}
