/***  File Header  ************************************************************/
/**
* cimg_nif.cc
*
* Elixir/Erlang extension module: CImg
* @author Shozo Fukuda
* @date	  Sun Dec 06 10:10:35 JST 2020
* System  MINGW64/Windows 10<br>
*
**/
/**************************************************************************{{{*/

#include <stdio.h>

//#define cimg_display    0
#include "CImgEx.h"
using namespace cimg_library;

/***** NIFs HELPER *****/
#include "my_erl_nif.h"

int enif_get_value(ErlNifEnv* env, ERL_NIF_TERM term, unsigned char* value)
{
    unsigned int temp;
    int res = enif_get_uint(env, term, &temp);
    *value = temp;
    return res;
}

int enif_get_value(ErlNifEnv* env, ERL_NIF_TERM term, int* value)
{
    int temp;
    int res = enif_get_int(env, term, &temp);
    *value = temp;
    return res;
}

int enif_get_value(ErlNifEnv* env, ERL_NIF_TERM term, float* value)
{
    double temp;
    int res = enif_get_double(env, term, &temp);
    *value = temp;
    return res;
}

ERL_NIF_TERM enif_make_value(ErlNifEnv* env, unsigned char value)
{
    return enif_make_uint(env, value);
}

ERL_NIF_TERM enif_make_value(ErlNifEnv* env, int value)
{
    return enif_make_int(env, value);
}

ERL_NIF_TERM enif_make_value(ErlNifEnv* env, float value)
{
    return enif_make_double(env, value);
}

int enif_get_color(ErlNifEnv* env, ERL_NIF_TERM term, unsigned char color[])
{
	int arity;
	const ERL_NIF_TERM* tuple3;
	unsigned int temp[3];

	if (!enif_get_tuple(env, term, &arity, &tuple3)
	||  arity != 3
	||  !enif_get_uint(env, tuple3[0], &temp[0])
	||  !enif_get_uint(env, tuple3[1], &temp[1])
	||  !enif_get_uint(env, tuple3[2], &temp[2])){
		color[0] = 0;
		color[1] = 0;
		color[2] = 0;
		return false;
	}

	color[0] = temp[0];
	color[1] = temp[1];
	color[2] = temp[2];

	return true;
}

int enif_get_pos(ErlNifEnv* env, ERL_NIF_TERM list, int val[3])
{
    unsigned int len;
    if (!enif_is_list(env, list)
    ||  !enif_get_list_length(env, list, &len)
    ||  !(len >= 1 && len <= 3)) {
        return false;
    }

    ERL_NIF_TERM item;
    unsigned int i;
    for (i = 0; i < len; i++) {
        if (!enif_get_list_cell(env, list, &item, &list)
        ||  !enif_get_int(env, item, &val[i])) {
            return false;
        }
    }
    for (; i < 3; i++) {
        val[i] = 0;
    }
    return true;
}


/***** Elixir.CImgDisplay.functions *****/
#if cimg_display != 0
#include "cimgdisplay_nif.h"
#endif

/***** Elixir.CImgDisplay.functions *****/
template <class T>
struct NifCImg {
    typedef CImg<T> CImgT;

    static void init_resource_type(ErlNifEnv* env, const char* name)
    {
        Resource<CImgT>::init_resource_type(env, name);
    }

    static int enif_get_image(ErlNifEnv* env, ERL_NIF_TERM term, CImgT** img)
    {
        ERL_NIF_TERM  key;
        ERL_NIF_TERM  handle;
        return enif_make_existing_atom(env, "handle", &key, ERL_NIF_LATIN1)
                && enif_get_map_value(env, term, key, &handle)
                && Resource<CImgT>::get_item(env, handle, img);
    }

    static ERL_NIF_TERM enif_make_image(ErlNifEnv* env, CImgT* img)
    {
        return Resource<CImgT>::make_resource(env, img, enif_make_list1(env,
            enif_make_tuple4(env,
                enif_make_int(env, img->width()),
                enif_make_int(env, img->height()),
                enif_make_int(env, img->depth()),
                enif_make_int(env, img->spectrum()))));
    }


    static DECL_NIF(create_scalar) {
        unsigned int size_x, size_y, size_z, size_c;
        T value;

        if (argc != 5
        ||  !enif_get_uint(env, argv[0], &size_x)
        ||  !enif_get_uint(env, argv[1], &size_y)
        ||  !enif_get_uint(env, argv[2], &size_z)
        ||  !enif_get_uint(env, argv[3], &size_c)
        ||  !enif_get_value(env, argv[4], &value)) {
            return enif_make_badarg(env);
        }

        CImgT* img;
        try {
            img = new CImgT(size_x, size_y, size_z, size_c, value);
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, img);
    }
    
    static DECL_NIF(create_from_f4bin) {
        unsigned int size_x, size_y, size_z, size_c;
        ErlNifBinary f4bin;

        if (argc != 5
        ||  !enif_get_uint(env, argv[0], &size_x)
        ||  !enif_get_uint(env, argv[1], &size_y)
        ||  !enif_get_uint(env, argv[2], &size_z)
        ||  !enif_get_uint(env, argv[3], &size_c)
        ||  !enif_inspect_binary(env, argv[4], &f4bin)
        ||  f4bin.size != size_x*size_y*size_z*size_c*sizeof(float)) {
            return enif_make_badarg(env);
        }

        CImgT* img;
        try {
            img = new CImgT(size_x, size_y, size_z, size_c);
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        float *bin = reinterpret_cast<float*>(f4bin.data);
        cimg_forXY(*img, x, y) {
        cimg_forC(*img, c) {
            (*img)(x, y, c) = static_cast<T>(256*(*bin++)+0.5);
        }}

        return enif_make_image(env, img);
    }

    static DECL_NIF(create_list) {
        unsigned int size_x, size_y, size_z, size_c;

        if (argc != 5
        ||  !enif_get_uint(env, argv[0], &size_x)
        ||  !enif_get_uint(env, argv[1], &size_y)
        ||  !enif_get_uint(env, argv[2], &size_z)
        ||  !enif_get_uint(env, argv[3], &size_c)
        ||  !enif_is_list(env, argv[4])) {
            return enif_make_badarg(env);
        }
        
        ERL_NIF_TERM list = argv[4];
        unsigned int len;
        if (!enif_get_list_length(env, list, &len)
        ||  len != size_x*size_y*size_z*size_c) {
            return enif_make_badarg(env);
        }

        CImgT* img;
        try {
            img = new CImgT(size_x, size_y, size_z, size_c);
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        cimg_for(*img,ptr,T) {
            ERL_NIF_TERM item;
            int val;
            if (!enif_get_list_cell(env, list, &item, &list)
            ||  !enif_get_int(env, item, &val)) {
                return enif_make_badarg(env);
            }
            *ptr = val;
        }

        return enif_make_image(env, img);
    }

    static DECL_NIF(create_copy) {
        CImgT* src;

        if (argc != 1
        ||  !enif_get_image(env, argv[0], &src)) {
            return enif_make_badarg(env);
        }

        CImgT* img;
        try {
            img = new CImgT(*src);
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, img);
    }

    static DECL_NIF(create_load) {
        ErlNifBinary bin;

        if (argc != 1
        ||  !enif_inspect_binary(env, argv[0], &bin)) {
            return enif_make_badarg(env);
        }
        std::string fname((const char*)bin.data, bin.size);

        CImgT* img;
        try {
            img = new CImgT(fname.c_str());
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, img);
    }
    
    static DECL_NIF(clear) {
        CImgT* img;

        if (argc != 1
        ||  !enif_get_image(env, argv[0], &img)) {
            return enif_make_badarg(env);
        }

        img->clear();
        
        return argv[0];
    }

    static DECL_NIF(save) {
        CImgT* img;
        ErlNifBinary bin;

        if (argc != 2
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_inspect_binary(env, argv[1], &bin)) {
            return enif_make_badarg(env);
        }

        std::string fname((const char*)bin.data, bin.size);

        img->save(fname.c_str());

        return enif_make_ok(env);
    }

    static DECL_NIF(shape) {
        CImgT* img;

        if (argc != 1
        ||  !enif_get_image(env, argv[0], &img)) {
            return enif_make_badarg(env);
        }

        return enif_make_tuple4(env,
            enif_make_int(env, img->width()),
            enif_make_int(env, img->height()),
            enif_make_int(env, img->depth()),
            enif_make_int(env, img->spectrum()));
    }

    static DECL_NIF(resize) {
        CImgT* img;
        int width, height;

        if (argc != 3
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_int(env, argv[1], &width)
        ||  !enif_get_int(env, argv[2], &height)) {
            return enif_make_badarg(env);
        }

        img->resize(width, height);

        return argv[0];
    }

    static DECL_NIF(get_resize) {
        CImgT* img;
        int width, height;

        if (argc != 3
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_int(env, argv[1], &width)
        ||  !enif_get_int(env, argv[2], &height)) {
            return enif_make_badarg(env);
        }

        CImgT* resize;
        try {
            resize = new CImgT(img->get_resize(width, height));
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, resize);
    }

    static DECL_NIF(get_packed) {
        CImgT* img;
        int width, height, fill;

        if (argc != 4
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_int(env, argv[1], &width)
        ||  !enif_get_int(env, argv[2], &height)
        ||  !enif_get_int(env, argv[3], &fill)) {
            return enif_make_badarg(env);
        }

        CImgT* packed;
        try {
            packed = new CImgT(width, height, 1, img->spectrum(), fill);

            double retio = std::min((double)width/img->width(), (double)height/img->height());
            packed->draw_image(img->get_resize(retio*img->width(), retio*img->height()));
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, packed);
    }

    static DECL_NIF(mirror) {
        CImgT* img;
        char axis[2];

        if (argc != 2
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_atom(env, argv[1], axis, 2, ERL_NIF_LATIN1)
        ||  (axis[0] != 'x' && axis[0] != 'y')) {
            return enif_make_badarg(env);
        }

        img->mirror(axis[0]);

        return argv[0];
    }

    static DECL_NIF(get_gray) {
        CImgT* img;
        int opt_pn;

        if (argc != 2
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_int(env, argv[1], &opt_pn)) {
            return enif_make_badarg(env);
        }

        CImgT* gray;
        try {
            gray = new CImgT(img->getRGBtoGRAY(opt_pn));
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, gray);
    }

    static DECL_NIF(blur) {
        CImgT* img;
        double sigma;
        bool   boundary_conditions;
        bool   is_gaussian;

        if (argc != 4
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_double(env, argv[1], &sigma)
        ||  !enif_get_bool(env, argv[2], &boundary_conditions)
        ||  !enif_get_bool(env, argv[3], &is_gaussian)) {
            return enif_make_badarg(env);
        }

        img->blur(sigma, boundary_conditions, is_gaussian);

        return argv[0];
    }

    static DECL_NIF(get_crop) {
        CImgT* img;
        int x0, y0, z0, c0;
        int x1, y1, z1, c1;
        unsigned int boundary_conditions;

        if (argc != 10
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_int(env, argv[1], &x0)
        ||  !enif_get_int(env, argv[2], &y0)
        ||  !enif_get_int(env, argv[3], &z0)
        ||  !enif_get_int(env, argv[4], &c0)
        ||  !enif_get_int(env, argv[5], &x1)
        ||  !enif_get_int(env, argv[6], &y1)
        ||  !enif_get_int(env, argv[7], &z1)
        ||  !enif_get_int(env, argv[8], &c1)
        ||  !enif_get_uint(env, argv[9], &boundary_conditions)) {
            return enif_make_badarg(env);
        }

        CImgT* crop;
        try {
            crop = new CImgT(img->get_crop(x0, y0, z0, c0, x1, y1, z1, c1, boundary_conditions));
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, crop);
    }

    static DECL_NIF(fill) {
        CImgT* img;
        T val;

        if (argc != 2
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_value(env, argv[1], &val)) {
            return enif_make_badarg(env);
        }

        img->fill(val);

        return argv[0];
    }

    static DECL_NIF(draw_graph) {
        CImgT* img;
        CImgT* data;
        unsigned char color[3];
        double opacity;
        unsigned int plot_type;
        int           vertex_type;
        double        ymin;
        double        ymax;
        unsigned int pattern;

        if (argc != 9
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_image(env, argv[1], &data)
        ||  !enif_get_color(env, argv[2], color)
        ||  !enif_get_number(env, argv[3], &opacity)
        ||  !enif_get_uint(env, argv[4], &plot_type)
        ||  !enif_get_int(env, argv[5], &vertex_type)
        ||  !enif_get_number(env, argv[6], &ymin)
        ||  !enif_get_number(env, argv[7], &ymax)
        ||  !enif_get_uint(env, argv[8], &pattern)) {
            return enif_make_badarg(env);
        }

        img->draw_graph(*data, color, opacity, plot_type, vertex_type, ymin, ymax, pattern);

        return argv[0];
    }

    static DECL_NIF(draw_circle_filled) {
        CImgT* img;
        int x0;
        int y0;
        int radius;
        unsigned char color[3];
        double opacity;

        if (argc != 6
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_int(env, argv[1], &x0)
        ||  !enif_get_int(env, argv[2], &y0)
        ||  !enif_get_int(env, argv[3], &radius)
        ||  !enif_get_color(env, argv[4], color)
        ||  !enif_get_number(env, argv[5], &opacity)) {
            return enif_make_badarg(env);
        }

        img->draw_circle(x0, y0, radius, color, opacity);

        return argv[0];
    }

    static DECL_NIF(draw_circle)
    {
        CImgT* img;
        int x0;
        int y0;
        int radius;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (argc != 7
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_int(env, argv[1], &x0)
        ||  !enif_get_int(env, argv[2], &y0)
        ||  !enif_get_int(env, argv[3], &radius)
        ||  !enif_get_color(env, argv[4], color)
        ||  !enif_get_number(env, argv[5], &opacity)
        ||  !enif_get_uint(env, argv[6], &pattern)) {
            return enif_make_badarg(env);
        }

        img->draw_circle(x0, y0, radius, color, opacity, pattern);

        return argv[0];
    }

    static DECL_NIF(display) {
#if cimg_display != 0
        CImgT* img;
        CImgDisplay* disp;

        if (argc != 2
        ||  !enif_get_image(env, argv[0], &img)
        ||  !NifCImgDisplay::enif_get_display(env, argv[1], &disp)) {
            return enif_make_badarg(env);
        }

        img->display(*disp);
#endif

        return argv[0];
    }

    static DECL_NIF(get_flat_u1) {
        CImgT* img;
        bool   nchw;    // to transpose NCHW
        bool   bgr;     // to convert RGB to BGR

        if (argc != 3
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_bool(env, argv[1], &nchw)
        ||  !enif_get_bool(env, argv[2], &bgr)) {
            return enif_make_badarg(env);
        }

        // select BGR convertion
        int color[4] = {0,1,2,3};
        if (bgr && img->spectrum() >= 3) {
            int tmp = color[0]; color[0] = color[2]; color[2] = tmp;
        }

        ERL_NIF_TERM binary;
        unsigned char* buff = enif_make_new_binary(env, img->size(), &binary);
        if (buff == NULL) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't alloc binary", ERL_NIF_LATIN1));
        }

        if (nchw) {
            cimg_forC(*img, c) {
            cimg_forXY(*img, x, y) {
                *buff++ = (*img)(x, y,  color[c]);
            }}
        }
        else {
            cimg_forXY(*img, x, y) {
            cimg_forC(*img, c) {
                *buff++ = (*img)(x, y,  color[c]);
            }}
        }

        return enif_make_tuple2(env, enif_make_ok(env), binary);
    }

    static DECL_NIF(get_flat_f4) {
        CImgT* img;
        bool   nchw;    // to transpose NCHW
        bool   bgr;     // to convert RGB to BGR
        bool   norm;    // to normalize

        if (argc != 4
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_bool(env, argv[1], &nchw)
        ||  !enif_get_bool(env, argv[2], &bgr)
        ||  !enif_get_bool(env, argv[3], &norm)) {
            return enif_make_badarg(env);
        }

        // select normalization
        double factor = 1.0;
        if (norm) {
            factor = 1.0/255.0;     // normalize into 0.0-1.0
        }

        // select BGR convertion
        int color[4] = {0,1,2,3};
        if (bgr && img->spectrum() >= 3) {
            int tmp = color[0]; color[0] = color[2]; color[2] = tmp;
        }

        ERL_NIF_TERM binary;
        float* buff = reinterpret_cast<float*>(enif_make_new_binary(env, 4*img->size(), &binary));
        if (buff == NULL) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't alloc binary", ERL_NIF_LATIN1));
        }

        if (nchw) {
            // NCHW
            cimg_forC(*img, c) {
            cimg_forXY(*img, x, y) {
                *buff++ = ((*img)(x, y, color[c])) * factor;
            }}
        }
        else {
            // NHWC
            cimg_forXY(*img, x, y) {
            cimg_forC(*img, c) {
                *buff++ = ((*img)(x, y, color[c])) * factor;
            }}
        }

        return enif_make_tuple2(env, enif_make_ok(env), binary);
    }

    static DECL_NIF(cimg_draw_box) {
        CImgT* img;
        double x0, y0, x1, y1;
        char color[3];

        int arity;
        const ERL_NIF_TERM* terms;

        if (argc != 6
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_double(env, argv[1], &x0)
        ||  !enif_get_double(env, argv[2], &y0)
        ||  !enif_get_double(env, argv[3], &x1)
        ||  !enif_get_double(env, argv[4], &y1)
        ||  !enif_get_tuple(env, argv[5], &arity, &terms)
        ||  arity != 3) {
            return enif_make_badarg(env);
        }

        for (int i = 0; i < arity; i++) {
            int tmp;
            enif_get_int(env, terms[i], &tmp);
            color[i] = tmp;
        }

        int width  = img->width();
        int height = img->height();

        int ix0 = x0*width;
        int iy0 = y0*height;
        int ix1 = x1*width;
        int iy1 = y1*height;

        img->draw_rectangle(ix0, iy0, ix1, iy1, color, 1, ~0U);

        return argv[0];
    }

    static DECL_NIF(cimg_draw_rectangle) {
        CImgT* img;
        int  x0, y0, x1, y1;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (argc != 6
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_int(env, argv[1], &x0)
        ||  !enif_get_int(env, argv[2], &y0)
        ||  !enif_get_int(env, argv[3], &x1)
        ||  !enif_get_int(env, argv[4], &y1)
        ||  !enif_get_color(env, argv[5], color)
        ||  !enif_get_number(env, argv[6], &opacity)
        ||  !enif_get_uint(env, argv[7], &pattern)) {
            return enif_make_badarg(env);
        }

        img->draw_rectangle(x0, y0, x1, y1, color, opacity, pattern);

        return argv[0];
    }

    static DECL_NIF(size) {
        CImgT* img;

        if (argc != 1
        ||  !enif_get_image(env, argv[0], &img)) {
            return enif_make_badarg(env);
        }

        return enif_make_ulong(env, img->size());
    }

    static DECL_NIF(transpose) {
        CImgT* img;

        if (argc != 1
        ||  !enif_get_image(env, argv[0], &img)) {
            return enif_make_badarg(env);
        }

        img->transpose();

        return argv[0];
    }

    static DECL_NIF(set) {
        CImgT* img;
        unsigned int x, y, z, c;
        T val;

        if (argc != 6
        ||  !enif_get_value(env, argv[0], &val)
        ||  !enif_get_image(env, argv[1], &img)
        ||  !enif_get_uint(env, argv[2], &x)
        ||  !enif_get_uint(env, argv[3], &y)
        ||  !enif_get_uint(env, argv[4], &z)
        ||  !enif_get_uint(env, argv[5], &c)) {
            return enif_make_badarg(env);
        }

        (*img)(x, y, z, c) = val;

        return argv[0];
    }

    static DECL_NIF(get) {
        CImgT* img;
        unsigned int x, y, z, c;
        T val;

        if (argc != 5
        ||  !enif_get_image(env, argv[0], &img)
        ||  !enif_get_uint(env, argv[1], &x)
        ||  !enif_get_uint(env, argv[2], &y)
        ||  !enif_get_uint(env, argv[3], &z)
        ||  !enif_get_uint(env, argv[4], &c)) {
            return enif_make_badarg(env);
        }

        val = (*img)(x, y, z, c);

        return enif_make_value(env, val);
    }

    static DECL_NIF(assign) {
        CImgT* dst;
        CImgT* src;

        if (argc != 2
        ||  !enif_get_image(env, argv[0], &dst)
        ||  !enif_get_image(env, argv[1], &src)){
            return enif_make_badarg(env);
        }

        *dst = *src;

        return argv[0];
    }

    static DECL_NIF(transfer) {
        CImgT* dst;
        CImgT* src;
        int cx, cy, cz;

        if (argc != 6
        ||  !enif_get_image(env, argv[0], &dst)
        ||  !enif_get_image(env, argv[1], &src)
        ||  !enif_is_list(env, argv[2])
        ||  !enif_get_int(env, argv[3], &cx)
        ||  !enif_get_int(env, argv[4], &cy)
        ||  !enif_get_int(env, argv[5], &cz)) {
            return enif_make_badarg(env);
        }

        ERL_NIF_TERM address = argv[2];
        ERL_NIF_TERM head;
        while (enif_get_list_cell(env, address, &head, &address)) {
            int ality;
            const ERL_NIF_TERM* pair;
            int q[3], p[3];
            if (!enif_get_tuple(env, head, &ality, &pair)
            ||  ality != 2
            ||  !enif_get_pos(env, pair[0], q)
            ||  !enif_get_pos(env, pair[1], p)) {
                continue;
            }

            q[0] += cx; q[1] += cy; q[2] += cz;
            p[0] += cx; p[1] += cy; p[2] += cz;

            if (dst->containsXYZC(q[0], q[1], q[2]) && src->containsXYZC(p[0], p[1], p[2])) {
                cimg_forC(*src, c) {
                    (*dst)(q[0], q[1], q[2], c) = (*src)(p[0], p[1], p[2], c);
                }
            }
        }

        return argv[0];
    }

    static DECL_NIF(transfer3) {
        CImgT* dst;
        CImgT* src;
        int p[3];
        CImg<int>* map;

        if (argc != 3
        ||  !enif_get_image(env, argv[0], &dst)
        ||  !enif_get_image(env, argv[1], &src)
        ||  !enif_get_pos(env, argv[2], p)
        ||  !NifCImg<int>::enif_get_image(env, argv[3], &map)){
            return enif_make_badarg(env);
        }

        ERL_NIF_TERM address = argv[3];
        ERL_NIF_TERM head;
        while (enif_get_list_cell(env, address, &head, &address)) {
            int ality;
            const ERL_NIF_TERM* pair;
            if (!enif_get_tuple(env, head, &ality, &pair)
            ||  ality != 2
            ||  !enif_is_list(env, pair[0])
            ||  !enif_is_list(env, pair[1])) {
                continue;
            }

            int q[3], p[3];
            if (!enif_get_pos(env, pair[0], q)
            ||  !enif_get_pos(env, pair[1], p)) {
                continue;
            }

            if (dst->containsXYZC(q[0], q[1], q[2]) && src->containsXYZC(p[0], p[1], p[2])) {
                cimg_forC(*src, c) {
                    (*dst)(q[0], q[1], q[2], c) = (*src)(p[0], p[1], p[2], c);
                }
            }
        }

        return argv[0];
    }
};


typedef NifCImg<unsigned char> NifCImgU8;
typedef NifCImg<int>            NifCImgMap;

int load(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM load_info)
{
    NifCImgU8::init_resource_type(env, "cimgu8");
    NifCImgMap::init_resource_type(env, "cimgmap");
#if cimg_display != 0
    NifCImgDisplay::init_resource_type(env, "cimgdisplay");
#endif
    return 0;
}

// Let's define the array of ErlNifFunc beforehand:
static ErlNifFunc nif_funcs[] = {
//  {erl_function_name, erl_function_arity, c_function, dirty_flags}
    {"cimg_create",           5, NifCImgU8::create_scalar,           0},
    {"cimg_create",           1, NifCImgU8::create_copy,             0},
    {"cimg_load",             1, NifCImgU8::create_load,             0},
    {"cimg_clear",            1, NifCImgU8::clear,                   0},
    {"cimg_save",             2, NifCImgU8::save,                    0},
    {"cimg_resize",           3, NifCImgU8::resize,                  0},
    {"cimg_get_resize",       3, NifCImgU8::get_resize,              0},
    {"cimg_get_packed",       4, NifCImgU8::get_packed,              0},
    {"cimg_mirror",           2, NifCImgU8::mirror,                  0},
    {"cimg_get_gray",         2, NifCImgU8::get_gray,                0},
    {"cimg_blur",             4, NifCImgU8::blur,                    0},
    {"cimg_get_crop",        10, NifCImgU8::get_crop,                0},
    {"cimg_fill",             2, NifCImgU8::fill,                    0},
    {"cimg_draw_graph",       9, NifCImgU8::draw_graph,              0},
    {"cimg_display",          2, NifCImgU8::display,                 0},
    {"cimg_set",              6, NifCImgU8::set,                     0},
    {"cimg_get",              5, NifCImgU8::get,                     0},
    {"cimg_assign",           2, NifCImgU8::assign,                  0},
    {"cimg_draw_circle",      6, NifCImgU8::draw_circle_filled,      0},
    {"cimg_draw_circle",      7, NifCImgU8::draw_circle,             0},
    {"cimg_shape",            1, NifCImgU8::shape,                   0},
    {"cimg_size",             1, NifCImgU8::size,                    0},
    {"cimg_get_flatbin",      3, NifCImgU8::get_flat_u1,             0},
    {"cimg_get_flatf4",       4, NifCImgU8::get_flat_f4,             0},
    {"cimg_draw_box",         6, NifCImgU8::cimg_draw_box,           0},
    {"cimg_transfer",         6, NifCImgU8::transfer,                0},
    {"cimg_from_f4bin",       5, NifCImgU8::create_from_f4bin,       0},

    {"cimgmap_create",        5, NifCImgMap::create_list,            0},
    {"cimgmap_set",           6, NifCImgMap::set,                    0},
    {"cimgmap_get",           5, NifCImgMap::get,                    0},

#if cimg_display != 0
    {"cimgdisplay_u8",        5, NifCImgDisplay::create<NifCImgU8>,  0},
    NIF_CIMG_DISPLAY_FUNCS
#endif
};

ERL_NIF_INIT(Elixir.CImgNIF, nif_funcs, load, NULL, NULL, NULL)

/*** cimg_nif.cc **********************************************************}}}*/