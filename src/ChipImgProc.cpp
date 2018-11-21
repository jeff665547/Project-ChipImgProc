#include <ChipImgProc/version.h>
#include <string>
#include <Nucleona/language.hpp>
namespace chipimgproc {
std::string get_version() {
    return UNWRAP_SYM_STR(ChipImgProc_VERSION);
}
}