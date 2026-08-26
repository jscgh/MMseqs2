#include <string>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <sstream>

#include "CgroupMemory.h"
#include "Debug.h"
#include "Util.h"

const char* binary_name = "test_util";

int main (int, const char**) {
    assert(SSTR((unsigned int)1) == "1");
    assert(SSTR((int)1) == "1");
    assert(SSTR((unsigned long long int)1) == "1");
    assert(SSTR((long long int)1) == "1");
    assert(SSTR((unsigned int)1) == "1");
    assert(SSTR((unsigned int)1) != "2");
    assert(SSTR('c') == "c");
    assert(SSTR(0.00314) == "3.140E-03");
    assert(SSTR((double)0.00314) == "3.140E-03");
    assert(SSTR("TEST") == "TEST");

    const size_t memoryPages = Util::getTotalMemoryPages();
    const size_t memoryPageSize = Util::getPageSize();
    const size_t physicalMemory = memoryPages > std::numeric_limits<size_t>::max() / memoryPageSize
                                  ? std::numeric_limits<size_t>::max()
                                  : memoryPages * memoryPageSize;
    const size_t effectiveMemory = Util::getTotalSystemMemory();
    assert(effectiveMemory > 0);
    assert(effectiveMemory <= physicalMemory);
    if (effectiveMemory == 0 || effectiveMemory > physicalMemory) {
        return EXIT_FAILURE;
    }

#ifdef __linux__
    std::istringstream mountInfo(
        "30 20 0:27 / /sys/fs/cgroup rw - cgroup2 cgroup rw\n"
        "31 20 0:28 /pbs /sys/fs/cgroup/memory rw - cgroup cgroup rw,memory\n"
        "32 20 0:29 / /sys/fs/cgroup/escaped\\040path rw - cgroup2 cgroup rw\n");
    const std::vector<CgroupMemory::Mount> mounts = CgroupMemory::parseMounts(mountInfo);
    assert(mounts.size() == 3);
    if (mounts.size() != 3 ||
        CgroupMemory::resolvePath(mounts[0], "/jobs/42") != "/sys/fs/cgroup/jobs/42" ||
        CgroupMemory::resolvePath(mounts[1], "/pbs/jobs/42") != "/sys/fs/cgroup/memory/jobs/42" ||
        !CgroupMemory::resolvePath(mounts[1], "/jobs/42").empty() ||
        CgroupMemory::resolveNamespacePath(mounts[1], "/") != "/sys/fs/cgroup/memory" ||
        CgroupMemory::resolveNamespacePath(mounts[1], "/jobs/42") != "/sys/fs/cgroup/memory/jobs/42" ||
        mounts[2].path != "/sys/fs/cgroup/escaped path" ||
        CgroupMemory::availableMemory(100, 95, 80) != 85 ||
        CgroupMemory::availableMemory(100, 120, 10) != 0) {
        return EXIT_FAILURE;
    }
#endif

    const size_t pageSize = Util::getPageSize();
    const size_t touchedSize = 5 * pageSize + 1;
    void *allocation = NULL;
    const int allocationResult = posix_memalign(&allocation, pageSize, touchedSize);
    assert(allocationResult == 0);
    if (allocationResult != 0) {
        return EXIT_FAILURE;
    }
    memset(allocation, 0, touchedSize);
    char *pages = static_cast<char *>(allocation);
    for (size_t page = 0; page <= 5; ++page) {
        pages[page * pageSize] = 1;
    }
    const char touchedPages = Util::touchMemory(pages, touchedSize);
    assert(touchedPages == 6);
    free(allocation);
    if (touchedPages != 6) {
        return EXIT_FAILURE;
    }

//    unsigned int sizes[5] = {1882, 150, 630, 929, 167};

//    for (size_t i = 0; i < 5; ++i) {
//        size_t start = 0;
//        size_t end = i;
//
//        Util::decomposeDomainByAminoAcid(3758, sizes, 5, i, 5, &start, &end);
//        std::cout << start << " " << end << std::endl;
//    }

    for (size_t i = 0; i < 5; ++i) {
        size_t start = 0;
        size_t end = i;

        Util::decomposeDomain(5, i, 5, &start, &end);
        std::cout << start << " " << end << std::endl;
    }
}
