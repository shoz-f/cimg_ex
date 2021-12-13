/***  File Header  ************************************************************/
/**
* cimgdisplay_nif.h
*
* Elixir/Erlang extension module: CImgDisplay functions
* @author Shozo Fukuda
* @date	  Wed Jul 07 16:36:26 JST 2021
* System  MINGW64/Windows 10, Ubuntu/WSL2<br>
*
**/
/**************************************************************************{{{*/

namespace NifCImgDisplay {

    /**********************************************************************}}}*/
    /* Resource handling                                                      */
    /**********************************************************************{{{*/
    void init_resource_type(ErlNifEnv* env, const char* name)
    {
        Resource<CImgDisplay>::init_resource_type(env, name);
    }

    int enif_get_display(ErlNifEnv* env, ERL_NIF_TERM term, CImgDisplay** img)
    {
        ERL_NIF_TERM  key;
        ERL_NIF_TERM  handle;
        return enif_make_existing_atom(env, "handle", &key, ERL_NIF_LATIN1)
                && enif_get_map_value(env, term, key, &handle)
                && Resource<CImgDisplay>::get_item(env, handle, img);
    }

    template <class T>
    _DECL_NIF(create) {
        typename T::CImgT* img;
        ErlNifBinary bin;
        unsigned int normalization;
        bool is_fullscreen;
        bool is_closed;

        if (ality != 5
        ||  !T::enif_get_image(env, term[0], &img)
        ||  !enif_inspect_binary(env, term[1], &bin)
        ||  !enif_get_uint(env, term[2], &normalization)
        ||  !enif_get_bool(env, term[3], &is_fullscreen)
        ||  !enif_get_bool(env, term[4], &is_closed)) {
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

        return Resource<CImgDisplay>::make_resource(env, display);
    }

    DECL_NIF(wait) {
        CImgDisplay *display;

        if (ality != 1
        ||  !enif_get_display(env, term[0], &display)) {
            return enif_make_badarg(env);
        }

        display->wait();

        return term[0];
    }

    DECL_NIF(wait_time) {
        CImgDisplay *display;
        unsigned int milliseconds;

        if (ality != 2
        ||  !enif_get_display(env, term[0], &display)
        ||  !enif_get_uint(env, term[1], &milliseconds)) {
            return enif_make_badarg(env);
        }

        display->wait(milliseconds);

        return term[0];
    }

    DECL_NIF(is_closed) {
        CImgDisplay* display;

        if (ality != 1
        ||  !enif_get_display(env, term[0], &display)) {
            return enif_make_badarg(env);
        }

        return (display->is_closed()) ? enif_make_true(env) : enif_make_false(env);
    }

    DECL_NIF(button) {
        CImgDisplay* display;

        if (ality != 1
        ||  !enif_get_display(env, term[0], &display)) {
            return enif_make_badarg(env);
        }

        return enif_make_uint(env, display->button());
    }

    DECL_NIF(mouse_y) {
        CImgDisplay* display;

        if (ality != 1
        ||  !enif_get_display(env, term[0], &display)) {
            return enif_make_badarg(env);
        }

        return enif_make_int(env, display->mouse_y());
    }
}

/*** cimgdisplay_nif.h ****************************************************}}}*/