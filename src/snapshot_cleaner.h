#ifndef SNAPSHOT_CLEANER_H
#define SNAPSHOT_CLEANER_H

#include <string>

// Function to detect the type of file system on a specific path
std::string detectFilesystemType(const std::string& path);

// Function to delete LVM snapshots
bool removeLVMSnapshots();

// Function to delete Btrfs snapshots from a specific snapshot directory
bool removeBtrfsSnapshots(const std::string& snapshotDir);

// Function to delete ZFS snapshots
bool removeZFSSnapshots();

// The main function is to clean up snapshots based on mount points.
bool cleanSnapshots(const std::string& mountPoint);

#endif // SNAPSHOT_CLEANER_H
