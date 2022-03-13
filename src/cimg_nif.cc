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
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "CImgEx.h"
using namespace cimg_library;

#include "my_erl_nif.h"

#include <map>

/**************************************************************************}}}*/
/* CImg helper: enif get color value                                          */
/**************************************************************************{{{*/
inline int enif_get_color(ErlNifEnv* env, ERL_NIF_TERM term, unsigned char color[])
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
inline int enif_get_pos(ErlNifEnv* env, ERL_NIF_TERM list, int val[3])
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
namespace NifCImgU8 {
    typedef CImg<unsigned char> CImgT;
    typedef int (*CmdCImg)(CImgT& img, ErlNifEnv*, int, const ERL_NIF_TERM[], ERL_NIF_TERM&);

    enum {
        CIMG_ERROR = 0,
        CIMG_SEED  = 1,
        CIMG_GROW  = 2,
        CIMG_CROP  = 3
    };

    /**********************************************************************}}}*/
    /* Resource handling                                                      */
    /**********************************************************************{{{*/
    void init_resource_type(ErlNifEnv* env, const char* name)
    {
        Resource<CImgT>::init_resource_type(env, name);
    }

    int enif_get_image(ErlNifEnv* env, ERL_NIF_TERM term, CImgT** img)
    {
        ERL_NIF_TERM  key;
        ERL_NIF_TERM  handle;
        return enif_make_existing_atom(env, "handle", &key, ERL_NIF_LATIN1)
                && enif_get_map_value(env, term, key, &handle)
                && Resource<CImgT>::get_item(env, handle, img);
    }

    ERL_NIF_TERM enif_make_image(ErlNifEnv* env, CImgT* img)
    {
        return Resource<CImgT>::make_resource(env, img);
    }
}

/***** CImg command implementation *****/
#define  CIMG_CMD(name) int cmd_##name(CImgT& img, ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[], ERL_NIF_TERM& res)
#define _CIMG_CMD(name) int cmd_##name(CImgT& img, ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[], ERL_NIF_TERM& res)

#include "cimg_cmd.h"

#undef   CIMG_CMD
#undef  _CIMG_CMD

namespace NifCImgU8 {
    /**********************************************************************}}}*/
    /* CImg command interpreter                                               */
    /**********************************************************************{{{*/
    const std::map<std::string, CmdCImg> _cmd_cimg = {
        #include "cimg_cmd.inc"
    };

    DECL_NIF(run) {
        if (ality != 1
        ||  !enif_is_list(env, term[0])) {
            return enif_make_badarg(env);
        }

        ERL_NIF_TERM script = term[0];
        ERL_NIF_TERM res;
        ERL_NIF_TERM cmd;
        CImgT img;

        while (enif_get_list_cell(env, script, &cmd, &script)) {
            int argc;
            const ERL_NIF_TERM* argv;
            if (!enif_get_tuple(env, cmd, &argc, &argv)) {
                return enif_make_badarg(env);
            }

            char name[40];
            if (argc < 1
            ||  !enif_get_atom(env, argv[0], name, sizeof(name), ERL_NIF_LATIN1)
            ||  _cmd_cimg.count(name) == 0) {
                return enif_make_badarg(env);
            }

            CmdCImg fn = _cmd_cimg.at(name);
            switch (fn(img, env, argc-1, &argv[1], res)) {
            case CIMG_ERROR:
                return res;
            case CIMG_SEED:
                break;
            case CIMG_GROW:
                break;
            case CIMG_CROP:
                return res;
            }
        }

        return enif_make_badarg(env);
    }
}

/***** Elixir.CImgDisplay.functions *****/
#if cimg_display != 0
#include "cimgdisplay_nif.h"
#endif

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
#endif
};

ERL_NIF_INIT(Elixir.CImg.NIF, nif_funcs, load, NULL, NULL, NULL)

/*** cimg_nif.cc **********************************************************}}}*/