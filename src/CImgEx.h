#ifndef cimg_plugin
#define cimg_plugin "CImgEx.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define cimg_load_plugin(filename) \
    if (!cimg::strcasecmp(ext,"jpg") \
    ||  !cimg::strcasecmp(ext,"jpeg") \
    ||  !cimg::strcasecmp(ext,"jpe") \
    ||  !cimg::strcasecmp(ext,"jfif") \
    ||  !cimg::strcasecmp(ext,"jif")) return load_stb_image_jpeg(filename);

#define cimg_save_plugin(filename) \
    if (!cimg::strcasecmp(ext,"jpg") \
    ||  !cimg::strcasecmp(ext,"jpeg") \
    ||  !cimg::strcasecmp(ext,"jpe") \
    ||  !cimg::strcasecmp(ext,"jfif") \
    ||  !cimg::strcasecmp(ext,"jif")) return save_stb_image_jpeg(filename);

#include "CImg.h"

#else
/**************************************************************************}}}*/
/*** CImg Plugins:                                                          ***/
/**************************************************************************{{{*/
CImg<T>& load_stb_image_jpeg(const char *const filename)
{
  int x, y, n;
  unsigned char* data = stbi_load(filename, &x, &y, &n, 0);
  if (data == NULL) {
    throw CImgIOException(_cimg_instance
                          "load_stb_image_jpeg: %s.",
                          cimg_instance, stbi_failure_reason());
  }

  try { assign(x, y, 1, n); } catch (...) { throw; }

  T *ptr_r = _data,
    *ptr_g = _data + 1UL*_width*_height,
    *ptr_b = _data + 2UL*_width*_height,
    *ptr_a = _data + 3UL*_width*_height;
  const unsigned char *ptrs = data;
  switch (_spectrum) {
  case 1:
    cimg_forXY(*this, x, y) {
      *(ptr_r++) = (T)*(ptrs++);
    }
    break;
  case 3:
    cimg_forXY(*this, x, y) {
      *(ptr_r++) = (T)*(ptrs++);
      *(ptr_g++) = (T)*(ptrs++);
      *(ptr_b++) = (T)*(ptrs++);
    }
    break;
  case 4:
    cimg_forXY(*this, x, y) {
      *(ptr_r++) = (T)*(ptrs++);
      *(ptr_g++) = (T)*(ptrs++);
      *(ptr_b++) = (T)*(ptrs++);
      *(ptr_a++) = (T)*(ptrs++);
    }
    break;
  }
  
  stbi_image_free(data);

  return *this;
}

const CImg<T>& save_stb_image_jpeg(const char *const filename, const unsigned int quality=100) const
{
  if (is_empty()) { return *this; }
  if (_depth > 1) {
    cimg::warn(_cimg_instance
               "save_stb_image_jpeg(): Instance is volumetric, only the first slice will be saved in file '%s'.",
               cimg_instance,
               filename);
  }

  unsigned char *buff = reinterpret_cast<unsigned char*>(malloc(_width*_height*_spectrum));
  if (buff == NULL) {
    throw CImgIOException(_cimg_instance
                           "save_stb_image_jpeg: Failed to allocate memory for work.",
                           cimg_instance);
  }

  unsigned char *ptrd = buff;
  switch (_spectrum) {
  case 1: {
      const T *ptr_g = data(0, 0, 0, 0);
      cimg_forXY(*this, x, y) {
        *(ptrd++) = (unsigned char)*(ptr_g++);
      }
    }
    break;
  case 3: {
      const T *ptr_r = data(0, 0, 0, 0),
               *ptr_g = data(0, 0, 0, 1),
               *ptr_b = data(0, 0, 0, 2);
      cimg_forXY(*this, x, y) {
        *(ptrd++) = (unsigned char)*(ptr_r++);
        *(ptrd++) = (unsigned char)*(ptr_g++);
        *(ptrd++) = (unsigned char)*(ptr_b++);
      }
    }
    break;
  case 4: {
      const T *ptr_r = data(0, 0, 0, 0),
               *ptr_g = data(0, 0, 0, 1),
               *ptr_b = data(0, 0, 0, 2),
               *ptr_a = data(0, 0, 0, 3);
      cimg_forXY(*this, x, y) {
        *(ptrd++) = (unsigned char)*(ptr_r++);
        *(ptrd++) = (unsigned char)*(ptr_g++);
        *(ptrd++) = (unsigned char)*(ptr_b++);
        *(ptrd++) = (unsigned char)*(ptr_a++);
      }
    }
    break;
  }
  
  stbi_write_jpg(filename, _width, _height, _spectrum, buff, quality);
  return *this;
}

// option: image convert POSI/NEGA 
enum {
    cPOSI = 0,
    cNEGA
};

// make a GRAY image
CImg<T> makeGRAY(int optPN=cPOSI)
{
    if (_spectrum != 3) {
        throw CImgInstanceException(_cimg_instance
                                    "makeGRAY(): Instance is not a RGB image.",
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
