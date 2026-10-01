#include <cstring>
#include <string>
#include <vector>

#include "Command.h"
#include "CommandDeclarations.h"
#include "DBReader.h"
#include "DBWriter.h"
#include "DownloadDatabase.h"
#include "Matcher.h"
#include "Parameters.h"
#include "Prefiltering.h"

const char* binary_name = "test_expandaln_prefetch";
extern const char* MMSEQS_CURRENT_INDEX_VERSION;
const char* index_version_compatible = MMSEQS_CURRENT_INDEX_VERSION;
std::vector<DatabaseDownload> externalDownloads;
std::vector<KmerThreshold> externalThreshold;
bool hide_base_downloads = false;
DEFAULT_PARAMETER_SINGLETON_INIT

namespace {

const char *QUERY_DB = "expandPrefetchQuery";
const char *TARGET_DB = "expandPrefetchTarget";
const char *RESULT_AB_DB = "expandPrefetchResultAb";
const char *RESULT_BC_DB = "expandPrefetchResultBc";
const char *AUTO_DB = "expandPrefetchAuto";
const char *FREAD_DB = "expandPrefetchFread";
const char *MMAP_DB = "expandPrefetchMmap";
const char *TOUCH_DB = "expandPrefetchTouch";

Command makeExpandalnCommand(Parameters &par) {
    return {
        "expandaln", expandaln, &par.expandaln, COMMAND_PROFILE_PROFILE,
        "Expand an alignment result based on another", NULL, NULL,
        "<queryDB> <targetDB> <resultDB> <resultDB> <alignmentDB>", 0,
        {
            {"queryDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::sequenceDb},
            {"targetDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::sequenceDb},
            {"resultDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::resultDb},
            {"resultDB", DbType::ACCESS_MODE_INPUT, DbType::NEED_DATA, &DbValidator::ppResultDb},
            {"alignmentDB", DbType::ACCESS_MODE_OUTPUT, DbType::NEED_DATA, &DbValidator::alignmentDb}
        }
    };
}

void removeDatabases() {
    DBReader<DBKeyType>::removeDb(QUERY_DB);
    DBReader<DBKeyType>::removeDb(TARGET_DB);
    DBReader<DBKeyType>::removeDb(RESULT_AB_DB);
    DBReader<DBKeyType>::removeDb(RESULT_BC_DB);
    DBReader<DBKeyType>::removeDb(AUTO_DB);
    DBReader<DBKeyType>::removeDb(FREAD_DB);
    DBReader<DBKeyType>::removeDb(MMAP_DB);
    DBReader<DBKeyType>::removeDb(TOUCH_DB);
}

void writeSequenceDb(const char *name, DBKeyType key, const char *sequence) {
    const std::string index = std::string(name) + ".index";
    DBWriter writer(name, index.c_str(), 1, Parameters::WRITER_ASCII_MODE,
                    Parameters::DBTYPE_AMINO_ACIDS);
    writer.open();
    writer.writeData(sequence, std::strlen(sequence), key);
    writer.close();
}

void writeResultDb(const char *name, DBKeyType queryKey, const Matcher::result_t &result) {
    const std::string index = std::string(name) + ".index";
    DBWriter writer(name, index.c_str(), 1, Parameters::WRITER_ASCII_MODE,
                    Parameters::DBTYPE_ALIGNMENT_RES);
    writer.open();
    char buffer[1024];
    const size_t length = Matcher::resultToBuffer(buffer, result, true, true);
    writer.writeData(buffer, length, queryKey);
    writer.close();
}

int runExpandaln(const Command &command, const char *output, int preloadMode) {
    Parameters &par = Parameters::getInstance();
    par.setDefaults();
    par.preloadMode = preloadMode;
    par.expandFilterClusters = 1;
    par.threads = 1;
    par.verbosity = 0;
    const char *arguments[] = {
        QUERY_DB, TARGET_DB, RESULT_AB_DB, RESULT_BC_DB, output
    };
    return expandaln(5, arguments, command);
}

bool databasesEqual(const char *lhsName, const char *rhsName) {
    const std::string lhsIndex = std::string(lhsName) + ".index";
    const std::string rhsIndex = std::string(rhsName) + ".index";
    DBReader<DBKeyType> lhs(lhsName, lhsIndex.c_str(), 1,
                            DBReader<DBKeyType>::USE_INDEX | DBReader<DBKeyType>::USE_DATA);
    DBReader<DBKeyType> rhs(rhsName, rhsIndex.c_str(), 1,
                            DBReader<DBKeyType>::USE_INDEX | DBReader<DBKeyType>::USE_DATA);
    lhs.open(DBReader<DBKeyType>::LINEAR_ACCCESS);
    rhs.open(DBReader<DBKeyType>::LINEAR_ACCCESS);

    bool equal = lhs.getSize() > 0 && lhs.getSize() == rhs.getSize();
    for (size_t i = 0; equal && i < lhs.getSize(); ++i) {
        equal = lhs.getDbKey(i) == rhs.getDbKey(i) &&
                lhs.getEntryLen(i) == rhs.getEntryLen(i) &&
                std::memcmp(lhs.getData(i, 0), rhs.getData(i, 0), lhs.getEntryLen(i)) == 0;
    }
    lhs.close();
    rhs.close();
    return equal;
}

} // namespace

int main(int, const char**) {
    removeDatabases();

    writeSequenceDb(QUERY_DB, 100, "ACDE");
    writeSequenceDb(TARGET_DB, 300, "ACDE");
    const Matcher::result_t resultAb(200, 20, 1.0f, 1.0f, 1.0f, 1e-20, 4,
                                     0, 3, 4, 0, 3, 4, "MMMM");
    const Matcher::result_t resultBc(300, 20, 1.0f, 1.0f, 1.0f, 1e-20, 4,
                                     0, 3, 4, 0, 3, 4, "MMMM");
    writeResultDb(RESULT_AB_DB, 100, resultAb);
    writeResultDb(RESULT_BC_DB, 200, resultBc);

    Parameters &par = Parameters::getInstance();
    const Command command = makeExpandalnCommand(par);
    const bool succeeded = runExpandaln(command, AUTO_DB, Parameters::PRELOAD_MODE_AUTO) == EXIT_SUCCESS &&
                           runExpandaln(command, FREAD_DB, Parameters::PRELOAD_MODE_FREAD) == EXIT_SUCCESS &&
                           runExpandaln(command, MMAP_DB, Parameters::PRELOAD_MODE_MMAP) == EXIT_SUCCESS &&
                           runExpandaln(command, TOUCH_DB, Parameters::PRELOAD_MODE_MMAP_TOUCH) == EXIT_SUCCESS;
    const bool equal = succeeded && databasesEqual(AUTO_DB, FREAD_DB) &&
                       databasesEqual(AUTO_DB, MMAP_DB) &&
                       databasesEqual(AUTO_DB, TOUCH_DB);

    removeDatabases();
    return equal ? EXIT_SUCCESS : EXIT_FAILURE;
}
