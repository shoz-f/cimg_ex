#include <erl_nif.h>
#include <string>

/***** NIFs HELPER *****/
#define DECL_NIF(name)  ERL_NIF_TERM name(ErlNifEnv* env, int argc, const ERL_NIF_TERM argv[])

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

int enif_get_bool(ErlNifEnv* env, ERL_NIF_TERM term, bool* cond)
{
    char atom[256];
    if (!enif_get_atom(env, term, atom, sizeof(atom), ERL_NIF_LATIN1)) {
        return false;
    }
    
    *cond = std::strcmp(atom, "false") != 0 && std::strcmp(atom, "nil") != 0;
    
    return true;
}

int enif_get_number(ErlNifEnv* env, ERL_NIF_TERM term, double* val)
{
    int ival;
    if (enif_get_int(env, term, &ival)) {
        *val = ival;
        return true;
    }
    
    return enif_get_double(env, term, val);
}

int enif_get_str(ErlNifEnv* env, ERL_NIF_TERM term, std::string* str)
{
    ErlNifBinary bin;
    if (!enif_inspect_binary(env, term, &bin)) {
        return false;
    }
    str->assign((const char*)bin.data, bin.size);

    return true;
}

/***** ERL RESOURCE HANDLING *****/
template <class T>
struct Resource {
    static ErlNifResourceType* _ResType;

    static void init_resource_type(ErlNifEnv* env, const char* name)
    {
        _ResType = enif_open_resource_type(env, NULL, name, destroy, ERL_NIF_RT_CREATE, NULL);
    }

    static void destroy(ErlNifEnv* env, void* ptr)
    {
        Resource<T>* res = reinterpret_cast<Resource<T>*>(ptr);
        if (!res->m_item) {
            delete res->m_item;
        }
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