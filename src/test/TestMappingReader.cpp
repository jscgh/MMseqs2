#include <cstdio>
#include <cstring>
#include <string>
#include <unistd.h>

#include "MappingReader.h"

const char* binary_name = "test_mappingreader";

int main(int, const char**) {
    char path[] = "legacy-mapping-XXXXXX";
    const int fd = mkstemp(path);
    if (fd == -1) {
        return EXIT_FAILURE;
    }
    FILE *file = fdopen(fd, "wb");
    if (file == NULL) {
        close(fd);
        return EXIT_FAILURE;
    }

    const char magic[] = {19, 0, 23, 12, 0};
    struct __attribute__((__packed__)) LegacyPair {
        uint32_t dbkey;
        TaxID taxon;
    };
    const LegacyPair entries[] = {{1, 11}, {7, 17}, {42, 99}};
    const bool written = fwrite(magic, sizeof(magic), 1, file) == 1 &&
                         fwrite(entries, sizeof(entries), 1, file) == 1;
    fclose(file);
    if (!written) {
        remove(path);
        return EXIT_FAILURE;
    }

    int status = EXIT_SUCCESS;
    {
        MappingReader reader(path, false);
        if (reader.lookup(1) != 11 || reader.lookup(7) != 17 ||
            reader.lookup(42) != 99 || reader.lookup(2) != 0) {
            status = EXIT_FAILURE;
        }
    }
    remove(path);
    return status;
}
