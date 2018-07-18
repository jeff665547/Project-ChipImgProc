#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/aruco.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
TEST(aruco, bin_to_int) {
    auto json_path = nucleona::test::data_dir() / "aruco_marker_layout.json";
    std::ifstream fin(json_path.string());
    nlohmann::json mk_layout;
    fin >> mk_layout;
    for( auto&& mk : mk_layout["marker_list"] ) {
        std::cout << chipimgproc::ArUco::bin_to_dec(mk) << std::endl;
    }
}