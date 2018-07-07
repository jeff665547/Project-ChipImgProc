#include <Nucleona/range.hpp>
#include <boost/filesystem.hpp>
namespace chipimgproc{

constexpr struct CvImRead{
    template<class PATHS>
    decltype(auto) operator()(const PATHS& paths, int flags = 1) const {
        return nucleona::range::transform(
            FWD(paths),
            [flags](auto&& path) {
                return cv::imread(path.string(), flags);
            }
        );
    }
} cv_imread;

constexpr struct CvImReadPipeOp {

    decltype(auto) operator()( int flags ) const {
        return nucleona::make_tuple(flags);
    }

} cv_imread_p;

}