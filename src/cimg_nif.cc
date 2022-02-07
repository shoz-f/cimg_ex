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
/* CImg helper: enif get color value                                          */
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

const unsigned char COLOR_CODE16[][3] = {
    { 255, 255, 255 },
    { 192, 192, 192 },
    { 128, 128, 128 },
    {   0,   0,   0 },
    { 255,   0,   0 },
    { 128,   0,   0 },
    { 255, 255,   0 },
    { 128, 128,   0 },
    {   0, 255,   0 },
    {   0, 128,   0 },
    {   0, 255, 255 },
    {   0, 128, 128 },
    {   0,   0, 255 },
    {   0,   0, 128 },
    { 255,   0, 255 },
    { 128,   0, 128 },
};

int enif_get_color_name(ErlNifEnv* env, ERL_NIF_TERM term, const unsigned char** color)
{
    char color_name[16];
    int len;
    
    if ((len = enif_get_atom(env, term, color_name, sizeof(color_name), ERL_NIF_LATIN1)) == 0) {
        return false;
    }

    if (std::strcmp(color_name, "white") == 0) {
        *color = COLOR_CODE16[ 0];
    }
    else if (std::strcmp(color_name, "silver") == 0) {
        *color = COLOR_CODE16[ 1];
    }
    else if (std::strcmp(color_name, "gray") == 0) {
        *color = COLOR_CODE16[ 2];
    }
    else if (std::strcmp(color_name, "black") == 0) {
        *color = COLOR_CODE16[ 3];
    }
    else if (std::strcmp(color_name, "red") == 0) {
        *color = COLOR_CODE16[ 4];
    }
    else if (std::strcmp(color_name, "maroon") == 0) {
        *color = COLOR_CODE16[ 5];
    }
    else if (std::strcmp(color_name, "yellow") == 0) {
        *color = COLOR_CODE16[ 6];
    }
    else if (std::strcmp(color_name, "olive") == 0) {
        *color = COLOR_CODE16[ 7];
    }
    else if (std::strcmp(color_name, "lime") == 0) {
        *color = COLOR_CODE16[ 8];
    }
    else if (std::strcmp(color_name, "green") == 0) {
        *color = COLOR_CODE16[ 9];
    }
    else if (std::strcmp(color_name, "aqua") == 0) {
        *color = COLOR_CODE16[10];
    }
    else if (std::strcmp(color_name, "teal") == 0) {
        *color = COLOR_CODE16[11];
    }
    else if (std::strcmp(color_name, "blue") == 0) {
        *color = COLOR_CODE16[12];
    }
    else if (std::strcmp(color_name, "navy") == 0) {
        *color = COLOR_CODE16[13];
    }
    else if (std::strcmp(color_name, "fuchsia") == 0) {
        *color = COLOR_CODE16[14];
    }
    else if (std::strcmp(color_name, "purple") == 0) {
        *color = COLOR_CODE16[15];
    }
    else if (std::strcmp(color_name, "transparent") == 0) {
        *color = 0;
    }
    else {
        return false;
    }

    return true;
}

/**************************************************************************}}}*/
/* CImg helper: enif get 3D position (vector)                                 */
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
    #include "cimg_nif.inc"

#if cimg_display != 0
    #include "cimgdisplay_nif.inc"
    #include "cimgdisplay_nif.ext"
#endif
};

ERL_NIF_INIT(Elixir.CImg.NIF, nif_funcs, load, NULL, NULL, NULL)

/*** cimg_nif.cc **********************************************************}}}*/