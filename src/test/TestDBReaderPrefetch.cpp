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
    if (reader.prefetchData(ids, 0) != 0) {
        return EXIT_FAILURE;
    }
    const size_t prefetchedBytes = reader.prefetchData(ids, 1);
    if (prefetchedBytes > 1) {
        return EXIT_FAILURE;
    }

    DBReader<DBKeyType> indexOnlyReader(
        "dataGap", "dataGap.index", 1, DBReader<DBKeyType>::USE_INDEX
    );
    indexOnlyReader.open(DBReader<DBKeyType>::NOSORT);
    if (indexOnlyReader.prefetchData(ids, SIZE_MAX) != 0) {
        return EXIT_FAILURE;
    }
    indexOnlyReader.close();

    DBReader<DBKeyType> freadReader(
        "dataGap", "dataGap.index", 1,
        DBReader<DBKeyType>::USE_INDEX | DBReader<DBKeyType>::USE_DATA |
            DBReader<DBKeyType>::USE_FREAD
    );
    freadReader.open(DBReader<DBKeyType>::NOSORT);
    if (freadReader.prefetchData(ids, SIZE_MAX) != 0) {
        return EXIT_FAILURE;
    }
    freadReader.close();

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

    DBWriter splitWriter(
        "prefetchSplit", "prefetchSplit.index", 2, 0,
        Parameters::DBTYPE_AMINO_ACIDS
    );
    splitWriter.open();
    splitWriter.writeData("AAAA", 4, 10, 0);
    splitWriter.writeData("BBBB", 4, 20, 1);
    splitWriter.close(false);

    DBReader<DBKeyType> splitReader(
        "prefetchSplit", "prefetchSplit.index", 1,
        DBReader<DBKeyType>::USE_INDEX | DBReader<DBKeyType>::USE_DATA
    );
    splitReader.open(DBReader<DBKeyType>::NOSORT);
    std::vector<size_t> splitIds;
    splitIds.push_back(splitReader.getId(20));
    splitIds.push_back(splitReader.getId(10));
    if (splitReader.getDataFileCnt() != 2 ||
        splitReader.prefetchData(splitIds, SIZE_MAX) != splitReader.getTotalDataSize()) {
        return EXIT_FAILURE;
    }
    splitReader.close();
    DBReader<DBKeyType>::removeDb("prefetchSplit");

    return EXIT_SUCCESS;
}
