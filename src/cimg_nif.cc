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

#include "CImgEx.h"
using namespace cimg_library;

/***  Module Header  ******************************************************}}}*/
/**
* NIFs helper
* @par description
*   NIFs helper functions regarding CImg
**/
/**************************************************************************{{{*/
#include "my_erl_nif.h"

/**************************************************************************}}}*/
/* Helper: enif get & make value override functions                           */
/**************************************************************************{{{*/
int enif_get_value(ErlNifEnv* env, ERL_NIF_TERM term, unsigned char* value)
{
    unsigned int temp;
    int res = enif_get_uint(env, term, &temp);
    *value = temp;
    return res;
}

int enif_get_value(ErlNifEnv* env, ERL_NIF_TERM term, int* value)
{
    return enif_get_int(env, term, value);
}

int enif_get_value(ErlNifEnv* env, ERL_NIF_TERM term, float* value)
{
    double temp;
    int res = enif_get_double(env, term, &temp);
    *value = temp;
    return res;
}

int enif_get_value(ErlNifEnv* env, ERL_NIF_TERM term, double* value)
{
    return enif_get_double(env, term, value);
}

ERL_NIF_TERM enif_make_value(ErlNifEnv* env, unsigned char value)
{
    return enif_make_uint(env, value);
}

ERL_NIF_TERM enif_make_value(ErlNifEnv* env, int value)
{
    return enif_make_int(env, value);
}

ERL_NIF_TERM enif_make_value(ErlNifEnv* env, double value)
{
    return enif_make_double(env, value);
}

/**************************************************************************}}}*/
/* Helper: enif get color value                                               */
/**************************************************************************{{{*/
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

/**************************************************************************}}}*/
/* Helper: enif get 3D position (vector)                                      */
/**************************************************************************{{{*/
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

/**************************************************************************}}}*/
/* CImg enif implementation                                                   */
/**************************************************************************{{{*/
/***** Elixir.CImgDisplay.functions *****/
#if cimg_display != 0
#include "cimgdisplay_nif.h"
#endif

/***** Elixir.CImg.functions *****/
#include "cimg_nif.h"

typedef NifCImg<unsigned char> NifCImgU8;

/**************************************************************************}}}*/
/* enif resource setup                                                        */
/**************************************************************************{{{*/
int load(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM load_info)
{
    NifCImgU8::init_resource_type(env, "cimg");

#if cimg_display != 0
    NifCImgDisplay::init_resource_type(env, "cimgdisplay");
#endif
    return 0;
}

/**************************************************************************}}}*/
/* enif function dispach table                                                */
/**************************************************************************{{{*/
static ErlNifFunc nif_funcs[] = {
//  {erl_function_name, erl_function_arity, c_function, dirty_flags}
// Creation:
    {"cimg_create",           5, NifCImgU8::create,                  0},
    {"cimg_from_bin",         6, NifCImgU8::create_from_bin,         0},
    {"cimg_dup",              1, NifCImgU8::duplicate,               0},
// I/O
    {"cimg_load",             1, NifCImgU8::load,                    0},
    {"cimg_save",             2, NifCImgU8::save,                    0},
    {"cimg_load_from_memory", 1, NifCImgU8::load_from_memory,        0},
    {"cimg_to_bin",           6, NifCImgU8::to_bin,                  0},
    {"cimg_display",          2, NifCImgU8::display,                 0},

    {"cimg_set",              6, NifCImgU8::set,                     0},
    {"cimg_get",              5, NifCImgU8::get,                     0},
    {"cimg_assign",           2, NifCImgU8::assign,                  0},
    {"cimg_fill",             2, NifCImgU8::fill,                    0},
    {"cimg_clear",            1, NifCImgU8::clear,                   0},
    {"cimg_shape",            1, NifCImgU8::shape,                   0},
    {"cimg_size",             1, NifCImgU8::size,                    0},
    {"cimg_resize",           3, NifCImgU8::resize,                  0},
    {"cimg_get_resize",       5, NifCImgU8::get_resize,              0},
    {"cimg_mirror",           2, NifCImgU8::mirror,                  0},
    {"cimg_transpose",        1, NifCImgU8::transpose,               0},
    {"cimg_threshold",        4, NifCImgU8::threshold,               0},
    {"cimg_get_gray",         2, NifCImgU8::get_gray,                0},
    {"cimg_blur",             4, NifCImgU8::blur,                    0},
    {"cimg_get_crop",        10, NifCImgU8::get_crop,                0},
// Drawing:
    {"cimg_draw_graph",       9, NifCImgU8::draw_graph,              0},
    {"cimg_draw_circle",      7, NifCImgU8::draw_circle,             0},
    {"cimg_draw_circle",      6, NifCImgU8::draw_circle_filled,      0},
    {"cimg_draw_rect",        8, NifCImgU8::draw_rectangle,          0},
    {"cimg_draw_rect",        7, NifCImgU8::draw_rectangle_filled,   0},
    {"cimg_draw_ratio_rect",  8, NifCImgU8::draw_ratio_rectangle,    0},
    {"cimg_draw_triangle",   10, NifCImgU8::draw_triangle,           0},
    {"cimg_draw_triangle",    9, NifCImgU8::draw_triangle_filled,    0},
    {"cimg_map",              3, NifCImgU8::map_color,               0},
    {"cimg_transfer",         6, NifCImgU8::transfer,                0},

#if cimg_display != 0
    {"cimgdisplay_u8",        5, NifCImgDisplay::create<NifCImgU8>,  0},
    NIF_CIMG_DISPLAY_FUNCS
#endif
};

ERL_NIF_INIT(Elixir.CImg.NIF, nif_funcs, load, NULL, NULL, NULL)

/*** cimg_nif.cc **********************************************************}}}*/