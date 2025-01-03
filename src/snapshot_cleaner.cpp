#include "snapshot_cleaner.h"
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

std::string detectFilesystemType(const std::string& path) {
    std::ostringstream command;
    command << "stat -f -c %T " << path << " 2>/dev/null";

    FILE* pipe = popen(command.str().c_str(), "r");
    if (!pipe) {
        return "";
    }

    char buffer[128];
    std::string result;
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
        result += buffer;
    }
    pclose(pipe);

    result.erase(result.find_last_not_of(" \n\r\t") + 1); // Trim newline
    return result;
}

bool removeLVMSnapshots() {
    if (system("lvs --noheadings -o lv_name,lv_attr | grep '^.*_snapshot' > snapshots.txt") != 0) {
        return false;
    }

    std::ifstream snapshotsFile("snapshots.txt");
    if (!snapshotsFile) {
        return false;
    }

    std::string snapshotName;
    while (std::getline(snapshotsFile, snapshotName)) {
        snapshotName = snapshotName.substr(0, snapshotName.find_first_of(" "));
        std::string command = "lvremove -f " + snapshotName;
        if (system(command.c_str()) != 0) {
            // Handle error silently
        }
    }

    snapshotsFile.close();
    return true;
}

bool removeBtrfsSnapshots(const std::string& snapshotDir) {
    try {
        for (const auto& entry : fs::directory_iterator(snapshotDir)) {
            if (fs::is_directory(entry)) {
                std::string command = "btrfs subvolume delete " + entry.path().string();
                if (system(command.c_str()) != 0) {
                    // Handle error silently
                }
            }
        }
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

bool removeZFSSnapshots() {
    if (system("zfs list -t snapshot -o name > zfs_snapshots.txt") != 0) {
        return false;
    }

    std::ifstream snapshotsFile("zfs_snapshots.txt");
    if (!snapshotsFile) {
        return false;
    }

    std::string snapshotName;
    while (std::getline(snapshotsFile, snapshotName)) {
        std::string command = "zfs destroy " + snapshotName;
        if (system(command.c_str()) != 0) {
            // Handle error silently
        }
    }

    snapshotsFile.close();
    return true;
}

bool cleanSnapshots(const std::string& mountPoint) {
    std::string fsType = detectFilesystemType(mountPoint);

    if (fsType.empty()) {
        return false;
    }

    if (fsType == "lvm2_member") {
        return removeLVMSnapshots();
    } else if (fsType == "btrfs") {
        return removeBtrfsSnapshots("/mnt/.snapshots");
    } else if (fsType == "zfs") {
        return removeZFSSnapshots();
    } else {
        return false;
    }
}
