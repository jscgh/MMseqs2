#include <string>
#include <iostream>
#include <cassert>
#include <cstdlib>
#include <cstring>

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
