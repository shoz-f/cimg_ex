/***  File Header  ************************************************************/
/**
* cimg_nif.cc
*
* Elixir/Erlang extension module: CImg
* @author	   Shozo Fukuda
* @date	create Sun Dec 06 10:10:35 JST 2020
* System	   MINGW64/Windows 10<br>
*
**/
/**************************************************************************{{{*/

#include <string>
#include <erl_nif.h>

#include "myCImg.h"
typedef CImg<unsigned char> CImgU8;

/***** MACRO *****/
#define enifOk(env)     enif_make_atom(env, "ok")
#define enifError(env)  enif_make_atom(env, "error")


/***** ERL RESOURCE HANDLING *****/
static ErlNifResourceType* _ResType_MyCImg = NULL;

struct MyCImg {
    //std::mutex   m_mutex;
    CImgU8*      m_img;
};

void cimg_destroy(ErlNifEnv* env, void* ptr)
{
    MyCImg* mycimg_ptr = reinterpret_cast<MyCImg*>(ptr);

    if (!mycimg_ptr->m_img) {
        delete mycimg_ptr->m_img;
    }
}

int load(ErlNifEnv *env, void **priv_data, ERL_NIF_TERM load_info)
{
    _ResType_MyCImg = enif_open_resource_type(env, NULL, "mycimg", cimg_destroy, ERL_NIF_RT_CREATE, NULL);
    return 0;
}


/***** ERL TERM CONVERTER *****/
int enif_get_cimgu8(ErlNifEnv* env, ERL_NIF_TERM term, CImgU8** cimgu8)
{
    ERL_NIF_TERM key;
    ERL_NIF_TERM handle;
    MyCImg* mycimg_ptr;

    if (enif_make_existing_atom(env, "handle", &key, ERL_NIF_LATIN1)
    &&  enif_get_map_value(env, term, key, &handle)
    &&  enif_get_resource(env, handle, _ResType_MyCImg, (void**)&mycimg_ptr)) {
        *cimgu8 = mycimg_ptr->m_img;
        return true;
    }
    else {
        return false;
    }
}

ERL_NIF_TERM enif_make_mycimg_resource(ErlNifEnv* env, CImgU8* cimgu8)
{
    MyCImg* mycimg_ptr = new(enif_alloc_resource(_ResType_MyCImg, sizeof(MyCImg))) MyCImg;
    if (!mycimg_ptr) {
        return enif_make_tuple2(env, enifError(env), enif_make_string(env, "Faild to allocate resource", ERL_NIF_LATIN1));
    }
    mycimg_ptr->m_img = cimgu8;

    ERL_NIF_TERM term = enif_make_resource(env, mycimg_ptr);
    enif_release_resource(mycimg_ptr);
    
    return enif_make_tuple2(env, enifOk(env), term);
}


/***** Elixir.CImg.functions *****/
ERL_NIF_TERM cimg_create(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    ErlNifBinary bin;

    if (argc != 1
    ||  !enif_inspect_binary(env, argv[0], &bin)) {
        return enif_make_badarg(env);
    }
    std::string fname((const char*)bin.data, bin.size);

    CImgU8* img;
    try {
        img = new CImgU8(fname.c_str());
    }
    catch (CImgException& e) {
        return enif_make_tuple2(env, enifError(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
    }

    return enif_make_mycimg_resource(env, img);
}

ERL_NIF_TERM cimg_save(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    CImgU8* img;
    ErlNifBinary bin;

    if (argc != 2
    ||  !enif_get_cimgu8(env, argv[0], &img)
    ||  !enif_inspect_binary(env, argv[1], &bin)) {
        return enif_make_badarg(env);
    }

    std::string fname((const char*)bin.data, bin.size);

    img->save(fname.c_str());

    return enifOk(env);
}

ERL_NIF_TERM cimg_get_wh(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    CImgU8* img;

    if (argc != 1
    ||  !enif_get_cimgu8(env, argv[0], &img)) {
        return enif_make_badarg(env);
    }

    int width  = img->width();
    int height = img->height();

    return enif_make_list2(env, enif_make_int(env, width), enif_make_int(env, height));
}

ERL_NIF_TERM cimg_resize(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    CImgU8* img;
    int width, height;

    if (argc != 3
    ||  !enif_get_cimgu8(env, argv[0], &img)
    ||  !enif_get_int(env, argv[1], &width)
    ||  !enif_get_int(env, argv[2], &height)) {
        return enif_make_badarg(env);
    }

    img->resize(width, height);

    return argv[0];
}

ERL_NIF_TERM cimg_mirror(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    CImgU8* img;
    char axis[2];

    if (argc != 2
    ||  !enif_get_cimgu8(env, argv[0], &img)
    ||  !enif_get_atom(env, argv[1], axis, 2, ERL_NIF_LATIN1)
    ||  (axis[0] != 'x' && axis[0] != 'y')) {
        return enif_make_badarg(env);
    }

    img->mirror(axis[0]);

    return argv[0];
}

ERL_NIF_TERM cimg_get_gray(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    CImgU8* img;
    int opt_pn;

    if (argc != 2
    ||  !enif_get_cimgu8(env, argv[0], &img)
    ||  !enif_get_int(env, argv[1], &opt_pn)) {
        return enif_make_badarg(env);
    }

    CImgU8* gray;
    try {
        gray = new CImgU8(img->getRGBtoGRAY(opt_pn));
    }
    catch (CImgException& e) {
        return enif_make_tuple2(env, enifError(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
    }

    return enif_make_mycimg_resource(env, gray);
}

ERL_NIF_TERM cimg_get_flatbin(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    CImgU8* img;

    if (argc != 1
    ||  !enif_get_cimgu8(env, argv[0], &img)) {
        return enif_make_badarg(env);
    }
    
    ERL_NIF_TERM binary;
    unsigned char* buff = enif_make_new_binary(env, img->size(), &binary);
    if (buff) {
        CImgU8& img_ref = *img;
        cimg_forXY(img_ref, x, y) {
        cimg_forC(img_ref, c) {
            *buff++ = img_ref(x, y, c);
        }}

        return enif_make_tuple2(env, enifOk(env), binary);
    }
    else {
        return enif_make_tuple2(env, enifError(env), enif_make_string(env, "can't alloc binary", ERL_NIF_LATIN1));
    }
}


ERL_NIF_TERM cimg_draw_box(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])
{
    CImgU8* img;
    double x0, y0, x1, y1;
    char color[3];
    
    int arity;
    const ERL_NIF_TERM* terms;

    if (argc != 6
    ||  !enif_get_cimgu8(env, argv[0], &img)
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

// Let's define the array of ErlNifFunc beforehand:
static ErlNifFunc nif_funcs[] = {
    // {erl_function_name, erl_function_arity, c_function, dirty_flags}
    {"cimg_create",      1, cimg_create,      0},
    {"cimg_save",        2, cimg_save,        0},
    {"cimg_get_wh",      1, cimg_get_wh,      0},
    {"cimg_resize",      3, cimg_resize,      0},
    {"cimg_mirror",      2, cimg_mirror,      0},
    {"cimg_get_gray",    2, cimg_get_gray,    0},
    {"cimg_get_flatbin", 1, cimg_get_flatbin, 0},
    {"cimg_draw_box",    6, cimg_draw_box,    0},
};

ERL_NIF_INIT(Elixir.CImg.NIF, nif_funcs, load, NULL, NULL, NULL)

/*** cimg_nif.cc **********************************************************}}}*/