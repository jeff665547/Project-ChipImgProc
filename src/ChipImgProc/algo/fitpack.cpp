#include <ChipImgProc/utils.h>
#include <cmath>
#include <fitpackpp/BSplineSurface.h>
#include <fitpack_fc.h>
#include <Nucleona/tuple.hpp>
#include <range/v3/algorithm/max.hpp>
#include <range/v3/algorithm/min.hpp>
#include <ChipImgProc/algo/fitpack.h>
#include <optional>

extern "C" {
    void fitpack_surfit(int *iopt, int *m, double *x, double *y, double *z, double *w, double *xb, double *xe, double *yb, double *ye, int *kx, int *ky,
                double *s, int *nxest, int *nyest, int *nmax, double *eps, int *nx, double *tx, int *ny, double *ty, double *c, double *fp,
                double *wrk1, int *lwrk1, double *wrk2, int *lwrk2, int *iwrk, int *kwrk, int *ier);
    void fitpack_bispev(double *tx, int *nx, double *ty, int *ny, double *c, int *kx, int *ky, double *x, int *mx, double *y, int *my, double *z,
                double *wrk, int *lwrk, int *iwrk, int *kwrk, int *ier);
    void fitpack_parder(double *tx, int *nx, double *ty, int *ny, double *c, int *kx, int *ky, int *nux, int *nuy, double *x, int *mx, double *y, int *my,
                double *z, double *wrk, int *lwrk, int *iwrk, int *kwrk, int *ier);
}

namespace chipimgproc::algo::fitpack {
std::tuple<
    std::vector<double>,
    std::vector<double>,
    std::vector<double>,
    std::vector<double>,
    int,
    double
> surfit(
    std::vector<double>& x,
    std::vector<double>& y,
    std::vector<double>& z,
    std::vector<double>& w,
    double xb, double xe, 
    double yb, double ye,
    int kx, int ky,
    int iopt, double s,
    double eps,
    int nxest, int nyest,
    int lwrk1, int lwrk2
) {
    double fp;
    int nx, ny, ier, nxo, nyo, lc;
    nx = ny = ier = nxo = nyo = 0;
    int m = x.size();
    auto nmax = nxest;
    if(nmax < nyest) {
        nmax = nyest;
    }
    int lcest = (nxest - kx - 1)*(nyest - ky - 1);
    int kwrk = m + (nxest - 2*kx - 1) * (nyest - 2*ky - 1);
    int lwa = 2*nmax + lcest + lwrk1 + lwrk2 + kwrk;
    auto wa = new double[lwa];
    double* tx = (double*)wa;
    double* ty = tx + nmax;
    double* c  = ty + nmax;
    double* wrk1 = c + lcest;
    int* iwrk = (int*)(wrk1 + lwrk1);
    double* wrk2 = ((double*)iwrk) + kwrk;

    fitpack_surfit(&iopt, &m, x.data(), y.data(), z.data(), w.data(), 
        &xb, &xe, &yb, &ye, &kx, &ky, &s, &nxest, &nyest, &nmax, &eps,
        &nx, tx, &ny, ty, c, &fp, wrk1, &lwrk1, wrk2, &lwrk2, iwrk, &kwrk,
        &ier
    );

    std::size_t i = 0;
    while( (ier > 10) && (i++ < 5)) {
        lwrk2 = ier;
        wrk2 = new double[lwrk2];
        fitpack_surfit(&iopt, &m, x.data(), y.data(), z.data(), w.data(), 
            &xb, &xe, &yb, &ye, &kx, &ky, &s, &nxest, &nyest, &nmax, &eps,
            &nx, tx, &ny, ty, c, &fp, wrk1, &lwrk1, wrk2, &lwrk2, iwrk, &kwrk,
            &ier
        );
        delete wrk2;
    }
    if(ier == 10) {
        throw std::runtime_error("invalid inputs");
    }
    lc = (nx - kx -1)*(ny - ky - 1);
    std::vector<double> _tx(tx, tx + nx);
    std::vector<double> _ty(ty, ty + ny);
    std::vector<double> _c(c, c + lc);
    std::vector<double> _wrk(wrk1, wrk1 + lc);
    // wa.clear();
    // std::free(wa);
    delete[] wa;
    return nucleona::make_tuple(
        std::move(_tx), std::move(_ty), 
        std::move(_c), std::move(_wrk), 
        std::move(ier), std::move(fp)
    );
}
std::tuple< 
    std::vector<double>, // tx
    std::vector<double>, // ty
    std::vector<double>  // c
> bisplrep(
    std::vector<double>& x,
    std::vector<double>& y,
    std::vector<double>& z,
    int k,
    std::optional<double> s
)
{
    const double eps = 1e-16;
    auto kx = k;
    auto ky = k;
    auto m = x.size();
    std::vector<double> w(m, 1);
    auto xb  = ranges::min(x);
    auto xe  = ranges::max(x);
    auto yb  = ranges::min(y);
    auto ye  = ranges::max(y);
    if(!s) s = m - std::sqrt(2*m);
    auto nxest = std::max(int(kx + std::sqrt(m/2)), 2*kx + 3);
    auto nyest = std::max(int(ky + std::sqrt(m/2)), 2*ky + 3);
    std::size_t wrk;
    auto u = nxest - kx - 1;
    auto v = nyest - ky - 1;
    auto km = std::max(kx, ky) + 1;
    auto ne = std::max(nxest, nyest);
    auto bx = kx * v + ky + 1;
    auto by = ky * u + kx + 1;
    auto b1 = bx;
    auto b2 = bx + v - ky;
    if(bx > by) {
        b1 = by;
        b2 = by + u - kx;
    }
    auto lwrk1 = u*v*( 2 + b1 + b2) + 
        2*(u + v + km*(m + ne) + ne - kx - ky) + 
        b2 + 1;
    auto lwrk2 = u*v*(b2 + 1) + b2;
    auto [tx, ty, c, _wrk, ier, fp] = surfit(x, y, z, w, xb, xe, yb, ye, kx, ky, 
        0, s.value(), eps, nxest, nyest, lwrk1, lwrk2);
    // auto ierm = std::min(11, std::max(-3, ier));
    return nucleona::make_tuple(
        std::move(tx), std::move(ty), 
        std::move(c)
    );
}

cv::Mat_<double> bisplev(
    std::vector<double>& tx,
    std::vector<double>& ty,
    std::vector<double>& c,
    int k,
    std::vector<double>& x,
    std::vector<double>& y
) {
    int nx = tx.size();
    int ny = ty.size();
    int mx = x.size();
    int my = y.size();
    int mxy = mx * my;
    int lwrk = x.size() * (k + 1) + y.size() * (k + 1);
    int kwrk = x.size() + y.size();
    int lwa = lwrk + kwrk;
    int ier;
    std::vector<double> wrk(lwrk);
    std::vector<int> iwrk(kwrk);
    cv::Mat_<double> z(mx, my);
    fitpack_bispev(tx.data(), &nx, ty.data(), &ny, c.data(), 
        &k, &k, x.data(), &mx, y.data(), &my, reinterpret_cast<double*>(z.data), 
        wrk.data(), &lwrk, iwrk.data(), &kwrk, &ier);
    if(ier == 10) {
        throw std::runtime_error("invalid input data");
    }
    if(ier) {
        throw std::runtime_error("bispev error");
    }
    return z;
}

}