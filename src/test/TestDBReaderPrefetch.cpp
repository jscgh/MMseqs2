#include <algorithm>
#include <cstring>
#include <vector>

#include "DBReader.h"
#include "DBWriter.h"
#include "Matcher.h"
#include "Parameters.h"

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
    if (!std::is_sorted(ids.begin(), ids.end(), [&reader](size_t lhs, size_t rhs) {
            return reader.getOffset(lhs) < reader.getOffset(rhs);
        }) || std::adjacent_find(ids.begin(), ids.end()) != ids.end()) {
        return EXIT_FAILURE;
    }
    const std::vector<size_t> constIds(ids);
    reader.prefetchData(constIds);
    const size_t prefetchedBytes = reader.prefetchData(ids, 1);
    if (prefetchedBytes > 1) {
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < reader.getSize(); ++i) {
        if (reader.getData(i, 0) == NULL || std::strlen(reader.getData(i, 0)) == 0) {
            return EXIT_FAILURE;
        }
    }

    DBWriter resultWriter(
        "prefetchResults", "prefetchResults.index", 1, 0,
        Parameters::DBTYPE_ALIGNMENT_RES
    );
    resultWriter.open();
    const char *firstResult = "111\tresult\n12\tresult\n111\tduplicate\n";
    const char *secondResult = "6\tresult\n999\tmissing\n2\tresult\n";
    resultWriter.writeData(firstResult, std::strlen(firstResult), 100, 0);
    resultWriter.writeData(secondResult, std::strlen(secondResult), 200, 0);
    resultWriter.close();

    DBReader<DBKeyType> resultReader(
        "prefetchResults", "prefetchResults.index", 1,
        DBReader<DBKeyType>::USE_INDEX | DBReader<DBKeyType>::USE_DATA
    );
    resultReader.open(DBReader<DBKeyType>::LINEAR_ACCCESS);
    Matcher::prefetchTargetData(resultReader, reader, 0, SIZE_MAX, 2);
    Matcher::prefetchTargetData(resultReader, reader, 1, 1, 0);
    if (Matcher::prefetchTargetData(resultReader, reader, 0, SIZE_MAX, 2, 1) > 1) {
        return EXIT_FAILURE;
    }
    resultReader.close();
    DBReader<DBKeyType>::removeDb("prefetchResults");

    reader.close();
    return EXIT_SUCCESS;
}
