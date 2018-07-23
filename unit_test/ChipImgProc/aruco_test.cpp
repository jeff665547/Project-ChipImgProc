#include <Nucleona/app/cli/gtest.hpp>
#include <Nucleona/test/data_dir.hpp>
#include <ChipImgProc/aruco.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
TEST(aruco, bin_to_int) {
    auto json_path = nucleona::test::data_dir() / "aruco_default_dict_6x6_250.json";
    std::ifstream fin(json_path.string());
    nlohmann::json mk_layout;
    fin >> mk_layout;
    for( auto&& mk : mk_layout["marker_list"] ) {
        std::cout << chipimgproc::ArUco::bin_to_dec(mk) << std::endl;
    }
    // int i = 0;
    // int j = 0;
    // for( auto&& mk : mk_layout["marker_list"] ) {
    //     std::stringstream num; 
    //     num << "\"" << chipimgproc::ArUco::bin_to_dec(mk) << "\"";


    //     std::cout 
    //         << std::setw(15) << num.str() 
    //         << ": " 
    //         << "[" << std::setw(2) << i << ", " << std::setw(2) << j << "],"
    //         << std::endl;
    //     j ++;
    //     if( j == 7 ) {
    //         i ++;
    //         j = 0;
    //     }
    //     if( i == 7 ) break;
    // }
}
TEST(aruco, int_to_mat ) {
    std::cout << chipimgproc::ArUco::dec_to_mat(8117912230, 6, 6) << std::endl;
}