/***  File Header  ************************************************************/
/**
* cimg_nif.cc
*
* Elixir/Erlang extension module: CImg
* @author	   Shozo Fukuda
* @date	load Sun Dec 06 10:10:35 JST 2020
* System	   MINGW64/Windows 10<br>
*
**/
/**************************************************************************{{{*/

//#define cimg_display    0
#include "CImgEx.h"
using namespace cimg_library;

#include "my_erl_nif.h"

/***** NIFs HELPER *****/
int enif_get_value(ErlNifEnv* env, ERL_NIF_TERM term, unsigned char* value)
{
    unsigned int temp;
    int res = enif_get_uint(env, term, &temp);
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
struct NifCImgDisplay {
    static void init_resource_type(ErlNifEnv* env, const char* name)
    {
        Resource<CImgDisplay>::init_resource_type(env, name);
    }

    static int enif_get_display(ErlNifEnv* env, ERL_NIF_TERM term, CImgDisplay** img)
    {
        ERL_NIF_TERM  key;
        ERL_NIF_TERM  handle;
        return enif_make_existing_atom(env, "handle", &key, ERL_NIF_LATIN1)
                && enif_get_map_value(env, term, key, &handle)
                && Resource<CImgDisplay>::get_item(env, handle, img);
    }

    template <class T>
    static ERL_NIF_TERM create(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        typename T::CImgT* img;
        ErlNifBinary bin;
        unsigned int normalization;
        bool is_fullscreen;
        bool is_closed;

        if (argc != 5
        ||  !T::enif_get_image(env, argv[0], &img)
        ||  !enif_inspect_binary(env, argv[1], &bin)
        ||  !enif_get_uint(env, argv[2], &normalization)
        ||  !enif_get_bool(env, argv[3], &is_fullscreen)
        ||  !enif_get_bool(env, argv[4], &is_closed)) {
            return enif_make_badarg(env);
        }
        std::string title((const char*)bin.data, bin.size);
        
        CImgDisplay* display;
        try {
            display = new CImgDisplay(*img, title.c_str(), normalization, is_fullscreen, is_closed);
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }
        
        return Resource<CImgDisplay>::make_resource(env, display, enif_make_list(env, 0));
    }
    
    static ERL_NIF_TERM wait(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgDisplay *display;

        if (argc != 1
        ||  !enif_get_display(env, argv[0], &display)) {
            return enif_make_badarg(env);
        }

        display->wait();
        
        return argv[0];
    }
    
    static ERL_NIF_TERM wait_time(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgDisplay *display;
        unsigned int milliseconds;

        if (argc != 2
        ||  !enif_get_display(env, argv[0], &display)
        ||  !enif_get_uint(env, argv[1], &milliseconds)) {
            return enif_make_badarg(env);
        }

        display->wait(milliseconds);
        
        return argv[0];
    }
    
    static ERL_NIF_TERM is_closed(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgDisplay* display;

        if (argc != 1
        ||  !enif_get_display(env, argv[0], &display)) {
            return enif_make_badarg(env);
        }
        
        return (display->is_closed()) ? enif_make_true(env) : enif_make_false(env);
    }
    
    static ERL_NIF_TERM button(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgDisplay* display;

        if (argc != 1
        ||  !enif_get_display(env, argv[0], &display)) {
            return enif_make_badarg(env);
        }
        
        return enif_make_uint(env, display->button());
    }

    static ERL_NIF_TERM mouse_y(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgDisplay* display;

        if (argc != 1
        ||  !enif_get_display(env, argv[0], &display)) {
            return enif_make_badarg(env);
        }
        
        return enif_make_int(env, display->mouse_y());
    }
};
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

    static ERL_NIF_TERM create(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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
    
    static ERL_NIF_TERM load(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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

    static ERL_NIF_TERM save(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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

    static ERL_NIF_TERM cimg_get_wh(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgT* img;

        if (argc != 1
        ||  !enif_get_image(env, argv[0], &img)) {
            return enif_make_badarg(env);
        }

        int width  = img->width();
        int height = img->height();

        return enif_make_list2(env, enif_make_int(env, width), enif_make_int(env, height));
    }

    static ERL_NIF_TERM cimg_get_whc(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgT* img;

        if (argc != 1
        ||  !enif_get_image(env, argv[0], &img)) {
            return enif_make_badarg(env);
        }

        int width    = img->width();
        int height   = img->height();
        int spectrum = img->spectrum();

        return enif_make_list3(env, enif_make_int(env, width), enif_make_int(env, height), enif_make_int(env, spectrum));
    }

    static ERL_NIF_TERM shape(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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
    
    static ERL_NIF_TERM resize(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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

    static ERL_NIF_TERM mirror(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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

    static ERL_NIF_TERM cimg_get_gray(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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
    
    static ERL_NIF_TERM blur(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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
    
    static ERL_NIF_TERM get_crop(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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

    static ERL_NIF_TERM fill(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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

    static ERL_NIF_TERM draw_graph(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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
    
    static ERL_NIF_TERM draw_circle_filled(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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

    static ERL_NIF_TERM draw_circle(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
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

    static ERL_NIF_TERM display(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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

    static ERL_NIF_TERM cimg_get_flat_u1(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgT* img;

        if (argc != 1
        ||  !enif_get_image(env, argv[0], &img)) {
            return enif_make_badarg(env);
        }

        ERL_NIF_TERM binary;
        unsigned char* buff = enif_make_new_binary(env, img->size(), &binary);
        if (buff) {
            CImgT& img_ref = *img;
            cimg_forXY(img_ref, x, y) {
            cimg_forC(img_ref, c) {
                *buff++ = img_ref(x, y, c);
            }}

            return enif_make_tuple2(env, enif_make_ok(env), binary);
        }
        else {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't alloc binary", ERL_NIF_LATIN1));
        }
    }

    static ERL_NIF_TERM cimg_get_flat_f4(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgT* img;

        if (argc != 1
        ||  !enif_get_image(env, argv[0], &img)) {
            return enif_make_badarg(env);
        }

        ERL_NIF_TERM binary;
        float* buff = reinterpret_cast<float*>(enif_make_new_binary(env, 4*img->size(), &binary));
        if (buff) {
            CImgT& img_ref = *img;
            cimg_forXY(img_ref, x, y) {
            cimg_forC(img_ref, c) {
                *buff++ = img_ref(x, y, c)/255.0;   // normalize into 0.0-1.0
            }}

            return enif_make_tuple2(env, enif_make_ok(env), binary);
        }
        else {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't alloc binary", ERL_NIF_LATIN1));
        }
    }

    static ERL_NIF_TERM cimg_draw_box(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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
    
    static ERL_NIF_TERM size(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgT* img;

        if (argc != 6
        ||  !enif_get_image(env, argv[0], &img)) {
          return enif_make_badarg(env);
        }
        
        return enif_make_ulong(env, img->size());
    }
    
    static ERL_NIF_TERM transpose(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgT* img;

        if (argc != 1
        ||  !enif_get_image(env, argv[0], &img)) {
          return enif_make_badarg(env);
        }
        
        img->transpose();

        return argv[0];
    }
    
    static ERL_NIF_TERM set(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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

    static ERL_NIF_TERM get(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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

    static ERL_NIF_TERM assign(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
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
    
    static ERL_NIF_TERM transfer(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
    {
        CImgT* dst;
        CImgT* src;

        if (argc != 3
        ||  !enif_get_image(env, argv[0], &dst)
        ||  !enif_get_image(env, argv[1], &src)
        ||  !enif_is_list(env, argv[2])){
            return enif_make_badarg(env);
        }

        ERL_NIF_TERM address = argv[2];
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
typedef NifCImg<float>          NifCImgF32;

int load(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM load_info)
{
    NifCImgU8::init_resource_type(env, "cimgu8");
#if cimg_display != 0
    NifCImgDisplay::init_resource_type(env, "cimgdisplay");
#endif
    return 0;
}

// Let's define the array of ErlNifFunc beforehand:
static ErlNifFunc nif_funcs[] = {
    // {erl_function_name, erl_function_arity, c_function, dirty_flags}
    {"cimg_create",           5, NifCImgU8::create,                  0},
    {"cimg_load",             1, NifCImgU8::load,                    0},
    {"cimg_save",             2, NifCImgU8::save,                    0},
    {"cimg_get_wh",           1, NifCImgU8::cimg_get_wh,             0},
    {"cimg_get_whc",          1, NifCImgU8::cimg_get_whc,            0},
    {"cimg_resize",           3, NifCImgU8::resize,                  0},
    {"cimg_mirror",           2, NifCImgU8::mirror,                  0},
    {"cimg_get_gray",         2, NifCImgU8::cimg_get_gray,           0},
    {"cimg_blur",             4, NifCImgU8::blur,                    0},
    {"cimg_get_crop",        10, NifCImgU8::get_crop,                0},
    {"cimg_fill",             2, NifCImgU8::fill,                    0},
    {"cimg_draw_graph",       9, NifCImgU8::draw_graph,              0},
    {"cimg_get_flatbin",      1, NifCImgU8::cimg_get_flat_u1,        0},
    {"cimg_get_flatnorm",     1, NifCImgU8::cimg_get_flat_f4,        0},
    {"cimg_draw_box",         6, NifCImgU8::cimg_draw_box,           0},
    {"cimg_display",          2, NifCImgU8::display,                 0},
    {"cimg_set",              6, NifCImgU8::set,                     0},
    {"cimg_get",              5, NifCImgU8::get,                     0},
    {"cimg_assign",           2, NifCImgU8::assign,                  0},
    {"cimg_draw_circle",      6, NifCImgU8::draw_circle_filled,      0},
    {"cimg_draw_circle",      7, NifCImgU8::draw_circle,             0},
    {"cimg_shape",            1, NifCImgU8::shape,                   0},
    {"cimg_transfer",         3, NifCImgU8::transfer,                0},

#if cimg_display != 0
    {"cimgdisplay_u8",        5, NifCImgDisplay::create<NifCImgU8>,  0},
    {"cimgdisplay_wait",      1, NifCImgDisplay::wait,               0},
    {"cimgdisplay_wait",      2, NifCImgDisplay::wait_time,          0},
    {"cimgdisplay_is_closed", 1, NifCImgDisplay::is_closed,          0},
    {"cimgdisplay_button",    1, NifCImgDisplay::button,             0},
    {"cimgdisplay_mouse_y",   1, NifCImgDisplay::mouse_y,            0},
#endif
};

ERL_NIF_INIT(Elixir.CImgNIF, nif_funcs, load, NULL, NULL, NULL)

/*** cimg_nif.cc **********************************************************}}}*/