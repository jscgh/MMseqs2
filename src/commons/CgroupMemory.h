#ifndef MMSEQS_CGROUPMEMORY_H
#define MMSEQS_CGROUPMEMORY_H

#include <istream>
#include <string>
#include <vector>

namespace CgroupMemory {

struct Mount {
    bool unified;
    std::string root;
    std::string path;
};

std::vector<Mount> parseMounts(std::istream &input);

std::string resolvePath(const Mount &mount, const std::string &hierarchyPath);

size_t availableMemory(size_t limit, size_t usage, size_t inactiveFile);

}

#endif
