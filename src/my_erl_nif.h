#include <erl_nif.h>
#include <string>

/***** NIFs HELPER *****/
#define MUT
#define  DECL_NIF(name)  ERL_NIF_TERM name(ErlNifEnv* env, int ality, const ERL_NIF_TERM term[])
#define _DECL_NIF(name)  ERL_NIF_TERM name(ErlNifEnv* env, int ality, const ERL_NIF_TERM term[])

/***  Module Header  ******************************************************}}}*/
/**
* make atom term
* @par description
*   make an atom term from name. if same atom is already existing, just return it.
*
* @return NIF term
**/
/**************************************************************************{{{*/
ERL_NIF_TERM enif_make_atom_ex(ErlNifEnv* env, const char* name)
{
    ERL_NIF_TERM res;
    if (enif_make_existing_atom(env, name, &res, ERL_NIF_LATIN1)) {
        return res;
    }
    else {
        return enif_make_atom(env, name);
    }
}
#define enif_make_true(env)     enif_make_atom_ex(env, "true")
#define enif_make_false(env)    enif_make_atom_ex(env, "false")
#define enif_make_ok(env)       enif_make_atom_ex(env, "ok")
#define enif_make_error(env)    enif_make_atom_ex(env, "error")

/***  Module Header  ******************************************************}}}*/
/**
* convert term to boolean
* @par description
*   convert the term to boolean. return false if the term is ":false" or ":nil".
*
* @return succeed or fail
**/
/**************************************************************************{{{*/
int enif_get_bool(ErlNifEnv* env, ERL_NIF_TERM term, bool* cond)
{
    char atom[256];
    int  len;
    if ((len = enif_get_atom(env, term, atom, sizeof(atom), ERL_NIF_LATIN1)) == 0) {
        return false;
    }
    
    *cond = (std::strcmp(atom, "false") != 0 && std::strcmp(atom, "nil") != 0);
    
    return true;
}

/***  Module Header  ******************************************************}}}*/
/**
* convert int or double term to double
* @par description
*   convert the number term to double.
*
* @return succeed or fail
**/
/**************************************************************************{{{*/
int enif_get_number(ErlNifEnv* env, ERL_NIF_TERM term, double* val)
{
    int ival;
    if (enif_get_int(env, term, &ival)) {
        *val = ival;
        return true;
    }
    
    return enif_get_double(env, term, val);
}

/***  Module Header  ******************************************************}}}*/
/**
* convert int or double term to int
* @par description
*   convert the number term to int.
*
* @return succeed or fail
**/
/**************************************************************************{{{*/
int enif_get_number(ErlNifEnv* env, ERL_NIF_TERM term, int* val)
{
    double fval;
    if (enif_get_double(env, term, &fval)) {
        *val = fval;
        return true;
    }
    
    return enif_get_int(env, term, val);
}

/***  Module Header  ******************************************************}}}*/
/**
* get & make value override functions
* @par description
*   get & make value override functions
*
* @return succeed or fail
**/
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

/***  Module Header  ******************************************************}}}*/
/**
* convert binary term to string
* @par description
*   convert the binary term to std::string.
*
* @return succeed or fail
**/
/**************************************************************************{{{*/
int enif_get_str(ErlNifEnv* env, ERL_NIF_TERM term, std::string* str)
{
    ErlNifBinary bin;
    if (!enif_inspect_binary(env, term, &bin)) {
        return false;
    }
    str->assign((const char*)bin.data, bin.size);

    return true;
}

/***  Class Header  *******************************************************}}}*/
/**
* Erl resouce handling
* @par description
*   wrapping Erl resouce with C++ struct.
**/
/**************************************************************************{{{*/
template <class T>
struct Resource {
    static ErlNifResourceType* _ResType;

    static void init_resource_type(ErlNifEnv* env, const char* name)
    {
        ErlNifResourceTypeInit init;
        init.dtor    = destroy;
        init.stop    = NULL;
        init.down    = NULL;
        init.members = 4;
        init.dyncall = NULL;

        _ResType = enif_init_resource_type(env, name, &init, ERL_NIF_RT_CREATE, NULL);
    }

    static void destroy(ErlNifEnv* env, void* ptr)
    {
        Resource<T>* res = reinterpret_cast<Resource<T>*>(ptr);
        if (!res->m_item) {
            delete res->m_item;
        }
    }

    static ERL_NIF_TERM make_resource(ErlNifEnv* env, T* item)
    {
        Resource<T>* res = new(enif_alloc_resource(_ResType, sizeof(Resource<T>))) Resource<T>;
        if (!res) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "Faild to allocate resource", ERL_NIF_LATIN1));
        }
        res->m_item = item;

        ERL_NIF_TERM term = enif_make_resource(env, res);
        enif_release_resource(res);

        return enif_make_tuple2(env, enif_make_ok(env), term);
    }

    static ERL_NIF_TERM make_resource(ErlNifEnv* env, T* item, ERL_NIF_TERM opts)
    {
        Resource<T>* res = new(enif_alloc_resource(_ResType, sizeof(Resource<T>))) Resource<T>;
        if (!res) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "Faild to allocate resource", ERL_NIF_LATIN1));
        }
        res->m_item = item;

        ERL_NIF_TERM term = enif_make_resource(env, res);
        enif_release_resource(res);

        return enif_make_tuple3(env, enif_make_ok(env), term, opts);
    }

    static int get_item(ErlNifEnv* env, ERL_NIF_TERM term, T** item)
    {
        Resource<T>* res;
        if (enif_get_resource(env, term, _ResType, (void**)&res)) {
            *item = res->m_item;
            return true;
        }
        else {
            return false;
        }
    }

    T* m_item;
};

template <class T>
ErlNifResourceType* Resource<T>::_ResType = NULL;

/*** end of my_erl_nif.h **************************************************}}}*/