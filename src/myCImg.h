#ifndef cimg_plugin

#define cimg_plugin     "myCImg.h"
#define cimg_display    0
#define cimg_use_jpeg
#include "CImg.h"
using namespace cimg_library;

#else
/**************************************************************************}}}*/
/*** CImg Plugins:                                                          ***/
/**************************************************************************{{{*/
// option: image convert POSI/NEGA 
enum {
    cPOSI = 0,
    cNEGA
};

// get a GRAY converted image
CImg<T> getRGBtoGRAY(int optPN=cPOSI)
{
    if (_spectrum != 3) {
        throw CImgInstanceException(_cimg_instance
                                    "getRGBtoGRAY(): Instance is not a RGB image.",
                                    cimg_instance);
    }
    CImg<T> res(width(), height(), depth(), 1);
    T *R = data(0,0,0,0), *G = data(0,0,0,1), *B = data(0,0,0,2), *Y = res.data(0,0,0,0);
    const longT whd = (longT)width()*height()*depth();
    cimg_pragma_openmp(parallel for cimg_openmp_if_size(whd,256))
    for (longT i = 0; i < whd; i++) {
        Y[i] = (T)(0.299f*R[i] + 0.587f*G[i] + 0.114f*B[i]);

        if (optPN == cNEGA) {
            Y[i] = cimg::type<T>::max() - Y[i];
        }
    }
    return res;
}
#endif
