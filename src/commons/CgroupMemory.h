#ifndef MMSEQS_CGROUPMEMORY_H
#define MMSEQS_CGROUPMEMORY_H

#include <cstddef>
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

std::size_t availableMemory(std::size_t limit, std::size_t usage, std::size_t inactiveFile);

}

#endif
