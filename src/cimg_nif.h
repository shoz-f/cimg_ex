/***  File Header  ************************************************************/
/**
* cimg_nif.h
*
* Elixir/Erlang extension module: CImg functions
* @author Shozo Fukuda
* @date	  Sun Dec 06 10:10:35 JST 2020
* System  MINGW64/Windows 10, Ubuntu/WSL2<br>
*
**/
/**************************************************************************{{{*/

inline void RGB2BGR(int rgb[3], int bgr[3])
{
    bgr[0] = rgb[2];    // B
    bgr[1] = rgb[1];    // G
    bgr[2] = rgb[0];    // R
}

inline void RGB2YUV(int rgb[3], int yuv[3])
{
}

template <class T>
struct NifCImg {

    typedef CImg<T> CImgT;

    /**********************************************************************}}}*/
    /* Resource handling                                                      */
    /**********************************************************************{{{*/
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
        return Resource<CImgT>::make_resource(env, img);
    }

    /**********************************************************************}}}*/
    /* Image creation functions                                               */
    /**********************************************************************{{{*/
    static DECL_NIF(create) {
        unsigned int size_x, size_y, size_z, size_c;
        T value;

        if (ality != 5
        ||  !enif_get_uint(env, term[0], &size_x)
        ||  !enif_get_uint(env, term[1], &size_y)
        ||  !enif_get_uint(env, term[2], &size_z)
        ||  !enif_get_uint(env, term[3], &size_c)
        ||  !enif_get_value(env, term[4], &value)) {
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

    static DECL_NIF(create_from_bin) {
        std::string dtype;
        unsigned int size_x, size_y, size_z, size_c;
        ErlNifBinary bin;

        if (ality != 6
        ||  !enif_inspect_binary(env, term[0], &bin)
        ||  !enif_get_uint(env, term[1], &size_x)
        ||  !enif_get_uint(env, term[2], &size_y)
        ||  !enif_get_uint(env, term[3], &size_z)
        ||  !enif_get_uint(env, term[4], &size_c)
        ||  !enif_get_str(env, term[5], &dtype)) {
            return enif_make_badarg(env);
        }

        CImgT* img;
        try {
            img = new CImgT(size_x, size_y, size_z, size_c);
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        if (dtype == "<f4" &&  bin.size == size_x*size_y*size_z*size_c*sizeof(float)) {
			float *p = reinterpret_cast<float*>(bin.data);
			cimg_forXY(*img, x, y) {
			cimg_forC(*img, c) {
				(*img)(x, y, c) = static_cast<T>(256*(*p++)+0.5);
			}}
		}
		else if (dtype == "<u1" &&  bin.size == size_x*size_y*size_z*size_c) {
			unsigned char *p = reinterpret_cast<unsigned char*>(bin.data);
			cimg_forXY(*img, x, y) {
			cimg_forC(*img, c) {
				(*img)(x, y, c) = static_cast<T>(*p++);
			}}
    	}
    	else {
            return enif_make_badarg(env);
    	}

        return enif_make_image(env, img);
    }

    static _DECL_NIF(create_list) {
        unsigned int size_x, size_y, size_z, size_c;

        if (ality != 5
        ||  !enif_get_uint(env, term[0], &size_x)
        ||  !enif_get_uint(env, term[1], &size_y)
        ||  !enif_get_uint(env, term[2], &size_z)
        ||  !enif_get_uint(env, term[3], &size_c)
        ||  !enif_is_list(env, term[4])) {
            return enif_make_badarg(env);
        }
        
        ERL_NIF_TERM list = term[4];
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

    static DECL_NIF(duplicate) {
        CImgT* src;

        if (ality != 1
        ||  !enif_get_image(env, term[0], &src)) {
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

    /**********************************************************************}}}*/
    /* load/save functions                                                    */
    /**********************************************************************{{{*/
    static DECL_NIF(load) {
        std::string fname;

        if (ality != 1
        ||  !enif_get_str(env, term[0], &fname)) {
            return enif_make_badarg(env);
        }

        CImgT* img;
        try {
            img = new CImgT(fname.c_str());
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, img);
    }

    static DECL_NIF(save) {
        CImgT* img;
        std::string fname;

        if (ality != 2
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_str(env, term[1], &fname)) {
            return enif_make_badarg(env);
        }

        img->save(fname.c_str());

        return enif_make_ok(env);
    }
    
    static DECL_NIF(load_from_memory) {
        ErlNifBinary bin;
        if (ality != 1
        ||  !enif_inspect_binary(env, term[0], &bin)) {
            return enif_make_badarg(env);
        }

        CImgT* img;
        try {
            img = new CImgT();
            img->load_from_memory((const unsigned char*)bin.data, bin.size);
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, img);
    }

    static DECL_NIF(convert_to) {
        CImgT* img;
        char format[5];

        if (ality != 2
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_atom(env, term[1], format, sizeof(format), ERL_NIF_LATIN1)
        ||  (std::strcmp(format, "jpeg") != 0 && std::strcmp(format, "png") != 0)) {
            return enif_make_badarg(env);
        }

        auto mem = img->save_to_memory(format);
        if (mem.size() == 0) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't convert empty image", ERL_NIF_LATIN1));
        }

        ERL_NIF_TERM binary;
        unsigned char* buff = enif_make_new_binary(env, mem.size(), &binary);
        if (buff == NULL) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't alloc binary", ERL_NIF_LATIN1));
        }

        std::memcpy(buff, mem.data(), mem.size());

        return enif_make_tuple2(env, enif_make_ok(env), binary);
    }

    /**********************************************************************}}}*/
    /* PIXEL operations                                                       */
    /**********************************************************************{{{*/
    static MUT DECL_NIF(set) {
        CImgT* img;
        unsigned int x, y, z, c;
        T val;

        if (ality != 6
        ||  !enif_get_value(env, term[0], &val)
        ||  !enif_get_image(env, term[1], &img)
        ||  !enif_get_uint(env, term[2], &x)
        ||  !enif_get_uint(env, term[3], &y)
        ||  !enif_get_uint(env, term[4], &z)
        ||  !enif_get_uint(env, term[5], &c)) {
            return enif_make_badarg(env);
        }

        (*img)(x, y, z, c) = val;

        return term[0];
    }

    static DECL_NIF(get) {
        CImgT* img;
        unsigned int x, y, z, c;
        T val;

        if (ality != 5
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_uint(env, term[1], &x)
        ||  !enif_get_uint(env, term[2], &y)
        ||  !enif_get_uint(env, term[3], &z)
        ||  !enif_get_uint(env, term[4], &c)) {
            return enif_make_badarg(env);
        }

        val = (*img)(x, y, z, c);

        return enif_make_value(env, val);
    }

    static _DECL_NIF(assign) {
        CImgT* dst;
        CImgT* src;

        if (ality != 2
        ||  !enif_get_image(env, term[0], &dst)
        ||  !enif_get_image(env, term[1], &src)){
            return enif_make_badarg(env);
        }

        *dst = *src;

        return term[0];
    }

    static MUT DECL_NIF(fill) {
        CImgT* img;
        T val;

        if (ality != 2
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_value(env, term[1], &val)) {
            return enif_make_badarg(env);
        }

        img->fill(val);

        return term[0];
    }

    static MUT DECL_NIF(clear) {
        CImgT* img;

        if (ality != 1
        ||  !enif_get_image(env, term[0], &img)) {
            return enif_make_badarg(env);
        }

        img->clear();
        
        return term[0];
    }

    /**********************************************************************}}}*/
    /* get Image attribute functions                                          */
    /**********************************************************************{{{*/
    static DECL_NIF(shape) {
        CImgT* img;

        if (ality != 1
        ||  !enif_get_image(env, term[0], &img)) {
            return enif_make_badarg(env);
        }

        return enif_make_tuple4(env,
            enif_make_int(env, img->width()),
            enif_make_int(env, img->height()),
            enif_make_int(env, img->depth()),
            enif_make_int(env, img->spectrum()));
    }

    static DECL_NIF(size) {
        CImgT* img;

        if (ality != 1
        ||  !enif_get_image(env, term[0], &img)) {
            return enif_make_badarg(env);
        }

        return enif_make_ulong(env, img->size());
    }

    static MUT _DECL_NIF(resize) {
        CImgT* img;
        int width, height;

        if (ality != 3
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_int(env, term[1], &width)
        ||  !enif_get_int(env, term[2], &height)) {
            return enif_make_badarg(env);
        }

        img->resize(width, height, -100, -100, 3);

        return term[0];
    }

    static DECL_NIF(get_resize) {
        CImgT* img;
        int width, height;
        int align;
        int filling;

        if (ality != 5
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_int(env, term[1], &width)
        ||  !enif_get_int(env, term[2], &height)
        ||  !enif_get_int(env, term[3], &align)
        ||  !enif_get_int(env, term[4], &filling)) {
            return enif_make_badarg(env);
        }

        CImgT* resize;
        try {
            if (align == 0) {
                resize = new CImgT(img->get_resize(width, height, -100, -100, 3));
            }
            else if (align == 1 || align == 2) {
                resize = new CImgT(width, height, img->depth(), img->spectrum(), filling);

                double ratio_w = (double)width/img->width();
                double ratio_h = (double)height/img->height();

                if (ratio_w <= ratio_h) {
                    // there is a gap in the vertical direction.
                    CImgT tmp_img(img->get_resize(width, ratio_w*img->height(), -100, -100, 3));

                    resize->draw_image(0, (align == 1) ? 0 : (height-tmp_img.height()), tmp_img);
                }
                else {
                    // there is a gap in the horizontal direction.
                    CImgT tmp_img(img->get_resize(ratio_h*img->width(), height, -100, -100, 3));

                    resize->draw_image((align == 1) ? 0 : (width-tmp_img.width()), 0, tmp_img);
                }
            }
            else {
                return enif_make_badarg(env);
            }
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, resize);
    }

    static MUT DECL_NIF(mirror) {
        CImgT* img;
        char axis[2];

        if (ality != 2
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_atom(env, term[1], axis, 2, ERL_NIF_LATIN1)
        ||  (axis[0] != 'x' && axis[0] != 'y')) {
            return enif_make_badarg(env);
        }

        img->mirror(axis[0]);

        return term[0];
    }

    static MUT DECL_NIF(transpose) {
        CImgT* img;

        if (ality != 1
        ||  !enif_get_image(env, term[0], &img)) {
            return enif_make_badarg(env);
        }

        img->transpose();

        return term[0];
    }

    static MUT DECL_NIF(threshold) {
        CImgT* img;
        T     value;
        bool  soft_threshold;
        bool  strict_threshold;

        if (ality != 4
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_value(env, term[1], &value)
        ||  !enif_get_bool(env, term[2], &soft_threshold)
        ||  !enif_get_bool(env, term[3], &strict_threshold)) {
            return enif_make_badarg(env);
        }

        img->threshold(value, soft_threshold, strict_threshold);

        return term[0];
    }

    static MUT DECL_NIF(get_threshold) {
        CImgT* img;
        T     value;
        bool  soft_threshold;
        bool  strict_threshold;

        if (ality != 4
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_value(env, term[1], &value)
        ||  !enif_get_bool(env, term[2], &soft_threshold)
        ||  !enif_get_bool(env, term[3], &strict_threshold)) {
            return enif_make_badarg(env);
        }

        CImgT* threshold;
        try {
            threshold = new CImgT(img->get_threshold(value, soft_threshold, strict_threshold));
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, threshold);
    }

    static DECL_NIF(get_gray) {
        CImgT* img;
        int opt_pn;

        if (ality != 2
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_int(env, term[1], &opt_pn)) {
            return enif_make_badarg(env);
        }

        CImgT* gray;
        try {
            gray = new CImgT(img->getGRAY(opt_pn));
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, gray);
    }

    static DECL_NIF(get_invert) {
        CImgT* img;
        int opt_pn;

        if (ality != 1
        ||  !enif_get_image(env, term[0], &img)) {
            return enif_make_badarg(env);
        }

        CImgT* inv;
        try {
            inv = new CImgT(img->operator~());
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, inv);
    }

    static DECL_NIF(get_yuv) {
        CImgT* img;

        if (ality != 1
        ||  !enif_get_image(env, term[0], &img)) {
            return enif_make_badarg(env);
        }

        CImgT* yuv;
        try {
            yuv = new CImgT(img->get_RGBtoYUV());
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }

        return enif_make_image(env, yuv);
    }

    static MUT DECL_NIF(blur) {
        CImgT* img;
        double sigma;
        bool   boundary_conditions;
        bool   is_gaussian;

        if (ality != 4
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_double(env, term[1], &sigma)
        ||  !enif_get_bool(env, term[2], &boundary_conditions)
        ||  !enif_get_bool(env, term[3], &is_gaussian)) {
            return enif_make_badarg(env);
        }

        img->blur(sigma, boundary_conditions, is_gaussian);

        return term[0];
    }

    static DECL_NIF(get_crop) {
        CImgT* img;
        int x0, y0, z0, c0;
        int x1, y1, z1, c1;
        unsigned int boundary_conditions;

        if (ality != 10
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_int(env, term[1], &x0)
        ||  !enif_get_int(env, term[2], &y0)
        ||  !enif_get_int(env, term[3], &z0)
        ||  !enif_get_int(env, term[4], &c0)
        ||  !enif_get_int(env, term[5], &x1)
        ||  !enif_get_int(env, term[6], &y1)
        ||  !enif_get_int(env, term[7], &z1)
        ||  !enif_get_int(env, term[8], &c1)
        ||  !enif_get_uint(env, term[9], &boundary_conditions)) {
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

    /**********************************************************************}}}*/
    /* drawing shape functions                                                */
    /**********************************************************************{{{*/
    static MUT DECL_NIF(draw_graph) {
        CImgT* img;
        CImgT* data;
        unsigned char color[3];
        double opacity;
        unsigned int plot_type;
        int           vertex_type;
        double        ymin;
        double        ymax;
        unsigned int pattern;

        if (ality != 9
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_image(env, term[1], &data)
        ||  !enif_get_color(env, term[2], color)
        ||  !enif_get_number(env, term[3], &opacity)
        ||  !enif_get_uint(env, term[4], &plot_type)
        ||  !enif_get_int(env, term[5], &vertex_type)
        ||  !enif_get_number(env, term[6], &ymin)
        ||  !enif_get_number(env, term[7], &ymax)
        ||  !enif_get_uint(env, term[8], &pattern)) {
            return enif_make_badarg(env);
        }

        img->draw_graph(*data, color, opacity, plot_type, vertex_type, ymin, ymax, pattern);

        return term[0];
    }

    static MUT DECL_NIF(draw_circle)
    {
        CImgT* img;
        int x0;
        int y0;
        int radius;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (ality != 7
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_int(env, term[1], &x0)
        ||  !enif_get_int(env, term[2], &y0)
        ||  !enif_get_int(env, term[3], &radius)
        ||  !enif_get_color(env, term[4], color)
        ||  !enif_get_number(env, term[5], &opacity)
        ||  !enif_get_uint(env, term[6], &pattern)) {
            return enif_make_badarg(env);
        }

        img->draw_circle(x0, y0, radius, color, opacity, pattern);

        return term[0];
    }

    static MUT DECL_NIF(draw_circle_filled) {
        CImgT* img;
        int x0;
        int y0;
        int radius;
        unsigned char color[3];
        double opacity;

        if (ality != 6
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_int(env, term[1], &x0)
        ||  !enif_get_int(env, term[2], &y0)
        ||  !enif_get_int(env, term[3], &radius)
        ||  !enif_get_color(env, term[4], color)
        ||  !enif_get_number(env, term[5], &opacity)) {
            return enif_make_badarg(env);
        }

        img->draw_circle(x0, y0, radius, color, opacity);

        return term[0];
    }

    static MUT DECL_NIF(draw_rectangle) {
        CImgT* img;
        int  x0, y0, x1, y1;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (ality != 8
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_int(env, term[1], &x0)
        ||  !enif_get_int(env, term[2], &y0)
        ||  !enif_get_int(env, term[3], &x1)
        ||  !enif_get_int(env, term[4], &y1)
        ||  !enif_get_color(env, term[5], color)
        ||  !enif_get_number(env, term[6], &opacity)
        ||  !enif_get_uint(env, term[7], &pattern)) {
            return enif_make_badarg(env);
        }

        img->draw_rectangle(x0, y0, x1, y1, color, opacity, pattern);

        return term[0];
    }

    static MUT DECL_NIF(draw_rectangle_filled) {
        CImgT* img;
        int  x0, y0, x1, y1;
        unsigned char color[3];
        double opacity;

        if (ality != 7
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_int(env, term[1], &x0)
        ||  !enif_get_int(env, term[2], &y0)
        ||  !enif_get_int(env, term[3], &x1)
        ||  !enif_get_int(env, term[4], &y1)
        ||  !enif_get_color(env, term[5], color)
        ||  !enif_get_number(env, term[6], &opacity)) {
            return enif_make_badarg(env);
        }

        img->draw_rectangle(x0, y0, x1, y1, color, opacity);

        return term[0];
    }

    static MUT DECL_NIF(draw_ratio_rectangle) {
        CImgT* img;
        double x0, y0, x1, y1;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (ality != 8
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_double(env, term[1], &x0)
        ||  !enif_get_double(env, term[2], &y0)
        ||  !enif_get_double(env, term[3], &x1)
        ||  !enif_get_double(env, term[4], &y1)
        ||  !enif_get_color(env, term[5], color)
        ||  !enif_get_number(env, term[6], &opacity)
        ||  !enif_get_uint(env, term[7], &pattern)) {
            return enif_make_badarg(env);
        }

        int width  = img->width();
        int height = img->height();

        int ix0 = x0*width;
        int iy0 = y0*height;
        int ix1 = x1*width;
        int iy1 = y1*height;

        img->draw_rectangle(ix0, iy0, ix1, iy1, color, opacity, pattern);

        return term[0];
    }

    static MUT DECL_NIF(draw_triangle) {
        CImgT* img;
        int  x0, y0, x1, y1, x2, y2;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (ality != 10
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_int(env, term[1], &x0)
        ||  !enif_get_int(env, term[2], &y0)
        ||  !enif_get_int(env, term[3], &x1)
        ||  !enif_get_int(env, term[4], &y1)
        ||  !enif_get_int(env, term[5], &x2)
        ||  !enif_get_int(env, term[6], &y2)
        ||  !enif_get_color(env, term[7], color)
        ||  !enif_get_number(env, term[8], &opacity)
        ||  !enif_get_uint(env, term[9], &pattern)) {
            return enif_make_badarg(env);
        }

        img->draw_triangle(x0, y0, x1, y1, x2, y2, color, opacity, pattern);

        return term[0];
    }

    static MUT DECL_NIF(draw_triangle_filled) {
        CImgT* img;
        int  x0, y0, x1, y1, x2, y2;
        unsigned char color[3];
        double opacity;

        if (ality != 9
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_int(env, term[1], &x0)
        ||  !enif_get_int(env, term[2], &y0)
        ||  !enif_get_int(env, term[3], &x1)
        ||  !enif_get_int(env, term[4], &y1)
        ||  !enif_get_int(env, term[5], &x2)
        ||  !enif_get_int(env, term[6], &y2)
        ||  !enif_get_color(env, term[7], color)
        ||  !enif_get_number(env, term[8], &opacity)) {
            return enif_make_badarg(env);
        }

        img->draw_triangle(x0, y0, x1, y1, x2, y2, color, opacity);

        return term[0];
    }

    static MUT _DECL_NIF(draw_image)
    {
        CImgT* img;
        CImgT* mask;
        double opacity;

        if (ality != 3
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_image(env, term[1], &mask)
        ||  !enif_get_number(env, term[2], &opacity)) {
            return enif_make_badarg(env);
        }

        img->draw_image(*mask, opacity);

    	return term[0];
    }
    
    static MUT DECL_NIF(draw_text)
    {
        CImgT* img;
        int x, y;
        std::string text;
        const unsigned char *fg_color, *bg_color;
        double opacity;
        unsigned int font_height;

        if (ality != 8
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_int(env, term[1], &x)
        ||  !enif_get_int(env, term[2], &y)
        ||  !enif_get_str(env, term[3], &text)
        ||  !enif_get_color_name(env, term[4], &fg_color)
        ||  !enif_get_color_name(env, term[5], &bg_color)
        ||  !enif_get_double(env, term[6], &opacity)
        ||  !enif_get_uint(env, term[7], &font_height)) {
            return enif_make_badarg(env);
        }

        img->draw_text(x, y, text.c_str(), fg_color, bg_color, opacity, font_height);

    	return term[0];
    }

    static DECL_NIF(color_mapping) {
        CImgT* img;
        char lut_name[8];
        unsigned int boundary_conditions;

        if (ality != 3
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_atom(env, term[1], lut_name, sizeof(lut_name), ERL_NIF_LATIN1)
        ||  !enif_get_uint(env, term[2], &boundary_conditions)) {
            return enif_make_badarg(env);
        }

        CImgT lut;
        if (std::strcmp(lut_name, "default")     == 0) { lut = CImgT::default_LUT256(); }
        else if (std::strcmp(lut_name, "lines") == 0) { lut = CImgT::lines_LUT256();   }
        else if (std::strcmp(lut_name, "hot")   == 0) { lut = CImgT::hot_LUT256();     }
        else if (std::strcmp(lut_name, "cool")  == 0) { lut = CImgT::cool_LUT256();    }
        else if (std::strcmp(lut_name, "jet")   == 0) { lut = CImgT::jet_LUT256();     }
        else {
        	return enif_make_badarg(env);
        }

        CImgT* map;
        try {
            map = new CImgT(img->get_map(lut, boundary_conditions));
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }
        
        return enif_make_image(env, map);
    }

    static DECL_NIF(color_mapping_by) {
        CImgT* img;
        unsigned int lut_length;
        char lut_name[8];
        unsigned int boundary_conditions;

        if (ality != 3
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_list_length(env, term[1], &lut_length)
        ||  !enif_get_uint(env, term[2], &boundary_conditions)) {
            return enif_make_badarg(env);
        }

        ERL_NIF_TERM list = term[1], item;
        unsigned char color[3];
        CImgT lut(lut_length, 1, 1, 3);
        for (int i = 0; i < lut_length; i++) {
            if (!enif_get_list_cell(env, list, &item, &list)
            ||  !enif_get_color(env, item, color)) {
                return enif_make_badarg(env);
            }
            lut(i, 0, 0, 0) = color[0];
            lut(i, 0, 0, 1) = color[1];
            lut(i, 0, 0, 2) = color[2];
        }

        CImgT* map;
        try {
            map = new CImgT(img->get_map(lut, boundary_conditions));
        }
        catch (CImgException& e) {
            return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
        }
        
        return enif_make_image(env, map);
    }

    static DECL_NIF(display) {
#if cimg_display != 0
        CImgT* img;
        CImgDisplay* disp;

        if (ality != 2
        ||  !enif_get_image(env, term[0], &img)
        ||  !NifCImgDisplay::enif_get_display(env, term[1], &disp)) {
            return enif_make_badarg(env);
        }

        img->display(*disp);
#endif

        return term[0];
    }

    static DECL_NIF(to_bin) {
        CImgT*      img;
        std::string dtype;
        double     lo, hi;
        bool        nchw;    // to transpose NCHW
        bool        bgr;     // to convert RGB to BGR

        if (ality != 6
        ||  !enif_get_image(env, term[0], &img)
        ||  !enif_get_str(env, term[1], &dtype)
        ||  !enif_get_double(env, term[2], &lo)
        ||  !enif_get_double(env, term[3], &hi)
        ||  !enif_get_bool(env, term[4], &nchw)
        ||  !enif_get_bool(env, term[5], &bgr)) {
            return enif_make_badarg(env);
        }

        // select BGR convertion
        int color[4] = {0,1,2,3};
        if (bgr && img->spectrum() >= 3) {
            int tmp = color[0]; color[0] = color[2]; color[2] = tmp;
        }

        ERL_NIF_TERM binary;
        if (dtype == "<f4") {
            float* buff = reinterpret_cast<float*>(enif_make_new_binary(env, 4*img->size(), &binary));
            if (buff == NULL) {
                return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't alloc binary", ERL_NIF_LATIN1));
            }
            
            // normalization coefficient
            double a = (hi - lo)/255.0;
            double b = lo;

            if (nchw) {
                cimg_forC(*img, c) cimg_forXY(*img, x, y) {
                    *buff++ = a*((*img)(x, y, color[c])) + b;
                }
            }
            else {
                cimg_forXY(*img, x, y) cimg_forC(*img, c) {
                    *buff++ = a*((*img)(x, y, color[c])) + b;
                }
            }
        }
        else {
            unsigned char* buff = enif_make_new_binary(env, img->size(), &binary);
            if (buff == NULL) {
                return enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't alloc binary", ERL_NIF_LATIN1));
            }

            if (nchw) {
                cimg_forC(*img, c) cimg_forXY(*img, x, y) {
                    *buff++ = (*img)(x, y,  color[c]);
                }
            }
            else {
                cimg_forXY(*img, x, y) cimg_forC(*img, c) {
                    *buff++ = (*img)(x, y,  color[c]);
                }
            }
        }

        return enif_make_tuple2(env, enif_make_ok(env), binary);
    }

    static DECL_NIF(transfer) {
        CImgT* dst;
        CImgT* src;
        int cx, cy, cz;

        if (ality != 6
        ||  !enif_get_image(env, term[0], &dst)
        ||  !enif_get_image(env, term[1], &src)
        ||  !enif_is_list(env, term[2])
        ||  !enif_get_int(env, term[3], &cx)
        ||  !enif_get_int(env, term[4], &cy)
        ||  !enif_get_int(env, term[5], &cz)) {
            return enif_make_badarg(env);
        }

        ERL_NIF_TERM address = term[2];
        ERL_NIF_TERM head;
        while (enif_get_list_cell(env, address, &head, &address)) {
            int count;
            const ERL_NIF_TERM* pair;
            int q[3], p[3];
            if (!enif_get_tuple(env, head, &count, &pair)
            ||  count != 2
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

        return term[0];
    }

    static MUT _DECL_NIF(transfer3) {
        CImgT* dst;
        CImgT* src;
        int p[3];
        CImg<int>* map;

        if (ality != 3
        ||  !enif_get_image(env, term[0], &dst)
        ||  !enif_get_image(env, term[1], &src)
        ||  !enif_get_pos(env, term[2], p)
        ||  !NifCImg<int>::enif_get_image(env, term[3], &map)){
            return enif_make_badarg(env);
        }

        ERL_NIF_TERM address = term[3];
        ERL_NIF_TERM head;
        while (enif_get_list_cell(env, address, &head, &address)) {
            int count;
            const ERL_NIF_TERM* pair;
            if (!enif_get_tuple(env, head, &count, &pair)
            ||  count != 2
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

        return term[0];
    }

    static DECL_NIF(runit);
};

/*** cimg_nif.h ***********************************************************}}}*/