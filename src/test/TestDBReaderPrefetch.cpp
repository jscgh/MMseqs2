#include <cstring>
#include <vector>

#include "DBReader.h"

const char* binary_name = "test_dbreader_prefetch";

int main(int, const char**) {
    DBReader<DBKeyType> reader(
        "dataGap", "dataGap.index", 1,
        DBReader<DBKeyType>::USE_INDEX | DBReader<DBKeyType>::USE_DATA
    );
    reader.open(DBReader<DBKeyType>::NOSORT);

    std::vector<size_t> ids;
    for (size_t i = reader.getSize(); i > 0; --i) {
        ids.push_back(i - 1);
    }
    ids.push_back(0);
    reader.prefetchData(ids);

    for (size_t i = 0; i < reader.getSize(); ++i) {
        if (reader.getData(i, 0) == NULL || std::strlen(reader.getData(i, 0)) == 0) {
            return EXIT_FAILURE;
        }
    }
    reader.close();
    return EXIT_SUCCESS;
}
