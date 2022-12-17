/***  File Header  ************************************************************/
/**
* cimg_cmd.h
*
* Elixir/Erlang extension module: CImg command functions
* @author Shozo Fukuda
* @date   Mon Feb 28 10:16:02 JST 2022
* System  MINGW64/Windows 10, Ubuntu/WSL2<br>
*
**/
/**************************************************************************{{{*/
#ifndef _CIMG_CMD_H
#define _CIMG_CMD_H

namespace NifCImgDisplay {
    int enif_get_display(ErlNifEnv*, ERL_NIF_TERM, CImgDisplay**);
}

namespace NifCImgU8 {
    /**********************************************************************}}}*/
    /* SEED: CImg creation command implementation                             */
    /**********************************************************************{{{*/
    CIMG_CMD(copy) {
        CImgT* origin;

        if (argc != 1
        ||  !enif_get_image(env, argv[0], &origin)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.assign(*origin);

        return CIMG_SEED;
    }

    CIMG_CMD(create) {
        unsigned int size_x, size_y, size_z, size_c;
        unsigned char value;

        if (argc != 5
        ||  !enif_get_uint(env, argv[0], &size_x)
        ||  !enif_get_uint(env, argv[1], &size_y)
        ||  !enif_get_uint(env, argv[2], &size_z)
        ||  !enif_get_uint(env, argv[3], &size_c)
        ||  !enif_get_value(env, argv[4], &value)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        try {
            img.assign(size_x, size_y, size_z, size_c, value);
        }
        catch (CImgException& e) {
            res = enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
            return CIMG_ERROR;
        }

        return CIMG_SEED;
    }

    CIMG_CMD(create_from_bin) {
        ErlNifBinary   bin;
        unsigned int size_x, size_y, size_z, size_c;
        std::string dtype;
        char conv_op[8];
        const ERL_NIF_TERM* conv_prms;
        int conv_prms_count = 2;
        bool nchw;     // from NCHW
        bool bgr;     // from BGR to RGB

        if (argc != 10
        ||  !enif_inspect_binary(env, argv[0], &bin)
        ||  !enif_get_uint(env, argv[1], &size_x)
        ||  !enif_get_uint(env, argv[2], &size_y)
        ||  !enif_get_uint(env, argv[3], &size_z)
        ||  !enif_get_uint(env, argv[4], &size_c)
        ||  !enif_get_str(env, argv[5], &dtype)
        ||  !enif_get_atom(env, argv[6], conv_op, sizeof(conv_op), ERL_NIF_LATIN1)
        ||  !enif_get_tuple(env, argv[7], &conv_prms_count, &conv_prms)
        ||  !enif_get_bool(env, argv[8], &nchw)
        ||  !enif_get_bool(env, argv[9], &bgr)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.assign(size_x, size_y, size_z, size_c);

        // select BGR convertion
        int color[4] = {0,1,2,3};
        if (bgr && img.spectrum() >= 3) {
            int tmp = color[0]; color[0] = color[2]; color[2] = tmp;
        }

        if (dtype == "<f4" &&  bin.size == size_x*size_y*size_z*size_c*sizeof(float)) {
            /* setup normalization converter **********************************/
            double a[4], b[4];
            if (strcmp(conv_op, "gauss") == 0 && conv_prms_count == 3) {
                for (int i = 0; i < conv_prms_count; i++) {
                    int stat_prms_count;
                    const ERL_NIF_TERM* stat_prms;
                    double mu, sigma;
                    if (!enif_get_tuple(env, conv_prms[i], &stat_prms_count, &stat_prms)
                    ||  stat_prms_count != 2
                    ||  !enif_get_double(env, stat_prms[0], &mu)
                    ||  !enif_get_double(env, stat_prms[1], &sigma)) {
                        res = enif_make_badarg(env);
                        return CIMG_ERROR;
                    }

                    a[color[i]] = sigma;
                    b[color[i]] = -mu/sigma;
                }
                a[3] = 255.0;
                b[3] = 0.0;
            }
            else if (strcmp(conv_op, "range") == 0 && conv_prms_count == 2) {
                double lo, hi;
                if (!enif_get_double(env, conv_prms[0], &lo)
                ||  !enif_get_double(env, conv_prms[1], &hi)) {
                    res = enif_make_badarg(env);
                    return CIMG_ERROR;
                }

                for (int i = 0; i < 3; i++) {
                    a[color[i]] = 255.0/(hi - lo);
                    b[color[i]] = lo;
                }
                a[3] = 255.0;
                b[3] = 0.0;
            }
            else {
                res = enif_make_badarg(env);
                return CIMG_ERROR;
            }

            auto convert = [a, b](int c, float x) {
                int y = static_cast<int>(a[c]*(x - b[c]) + 0.5);
                return (y < 0) ? 0 : (y > 255) ? 255 : y;
            };
            /* ****************************************************************/

            float *p = reinterpret_cast<float*>(bin.data);

            if (nchw) {
                cimg_forC(img, c) cimg_forXY(img, x, y) {
                    img(x, y, color[c]) = convert(c, *p++);
                }
            }
            else {
                cimg_forXY(img, x, y) cimg_forC(img, c) {
                    img(x, y, color[c]) = convert(c, *p++);
                }
            }
        }
        else if (dtype == "<u1" &&  bin.size == size_x*size_y*size_z*size_c) {
            unsigned char *p = reinterpret_cast<unsigned char*>(bin.data);
            if (nchw) {
                cimg_forC(img, c) cimg_forXY(img, x, y) {
                    img(x, y, color[c]) = static_cast<unsigned char>(*p++);
                }
            }
            else {
                cimg_forXY(img, x, y) cimg_forC(img, c) {
                    img(x, y, color[c]) = static_cast<unsigned char>(*p++);
                }
            }
        }
        else {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        return CIMG_SEED;
    }

    CIMG_CMD(load) {
        std::string fname;

        if (argc != 1
        ||  !enif_get_str(env, argv[0], &fname)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        try {
            img.assign(fname.c_str());
        }
        catch (CImgException& e) {
            res = enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
            return CIMG_ERROR;
        }

        return CIMG_SEED;
    }

    CIMG_CMD(load_from_memory) {
        ErlNifBinary bin;
        if (argc != 1
        ||  !enif_inspect_binary(env, argv[0], &bin)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        try {
            img.load_from_memory((const unsigned char*)bin.data, bin.size);
        }
        catch (CImgException& e) {
            res = enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
            return CIMG_ERROR;
        }

        return CIMG_SEED;
    }

    /**********************************************************************}}}*/
    /* GROW: CImg processing command implementation                           */
    /**********************************************************************{{{*/
    CIMG_CMD(clear) {
        if (argc != 0) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.clear();

        return CIMG_GROW;
    }

    CIMG_CMD(fill) {
        unsigned char val;

        if (argc != 1
        ||  !enif_get_value(env, argv[0], &val)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.fill(val);

        return CIMG_GROW;
    }

    CIMG_CMD(invert) {
        if (argc != 0) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        cimg_for(img, ptr, unsigned char) { *ptr ^= (unsigned char)(-1); }

        return CIMG_GROW;
    }

    CIMG_CMD(gray) {
        int opt_pn;

        if (argc != 1
        ||  !enif_get_int(env, argv[0], &opt_pn)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        try {
            img.RGBtoGRAY(opt_pn);
        }
        catch (CImgException& e) {
            res = enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
            return CIMG_ERROR;
        }

        return CIMG_GROW;
    }

    CIMG_CMD(threshold) {
        unsigned char value;
        bool soft_threshold;
        bool strict_threshold;

        if (argc != 3
        ||  !enif_get_value(env, argv[0], &value)
        ||  !enif_get_bool(env, argv[1], &soft_threshold)
        ||  !enif_get_bool(env, argv[2], &strict_threshold)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.threshold(value, soft_threshold, strict_threshold);

        return CIMG_GROW;
    }

    CIMG_CMD(blend) {
        CImgT* mask;
        double ratio;

        if (argc != 2
        ||  !enif_get_image(env, argv[0], &mask)
        ||  !enif_get_number(env, argv[1], &ratio)
        ||  !(ratio >= 0.0 && ratio <= 1.0)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img = (1.0 - ratio)*img + ratio*(*mask);

        return CIMG_GROW;
    }

    CIMG_CMD(color_mapping) {
        char lut_name[8];
        unsigned int boundary_conditions;

        if (argc != 2
        ||  !enif_get_atom(env, argv[0], lut_name, sizeof(lut_name), ERL_NIF_LATIN1)
        ||  !enif_get_uint(env, argv[1], &boundary_conditions)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        CImgT lut;
        if (std::strcmp(lut_name, "default")     == 0) { lut = CImgT::default_LUT256(); }
        else if (std::strcmp(lut_name, "lines") == 0) { lut = CImgT::lines_LUT256();   }
        else if (std::strcmp(lut_name, "hot")   == 0) { lut = CImgT::hot_LUT256();     }
        else if (std::strcmp(lut_name, "cool")  == 0) { lut = CImgT::cool_LUT256();    }
        else if (std::strcmp(lut_name, "jet")   == 0) { lut = CImgT::jet_LUT256();     }
        else {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.map(lut, boundary_conditions);

        return CIMG_GROW;
    }

    CIMG_CMD(color_mapping_by) {
        unsigned int lut_length;
        unsigned int boundary_conditions;

        if (argc != 2
        ||  !enif_get_list_length(env, argv[0], &lut_length)
        ||  !enif_get_uint(env, argv[1], &boundary_conditions)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        ERL_NIF_TERM list = argv[0], item;
        unsigned char color[3];
        CImgT lut(lut_length, 1, 1, 3);
        for (int i = 0; i < lut_length; i++) {
            if (!enif_get_list_cell(env, list, &item, &list)
            ||  !enif_get_color(env, item, color)) {
                res = enif_make_badarg(env);
                return CIMG_ERROR;
            }
            lut(i, 0, 0, 0) = color[0];
            lut(i, 0, 0, 1) = color[1];
            lut(i, 0, 0, 2) = color[2];
        }

        img.map(lut, boundary_conditions);

        return CIMG_GROW;
    }

    CIMG_CMD(blur) {
        double sigma;
        bool   boundary_conditions;
        bool   is_gaussian;

        if (argc != 3
        ||  !enif_get_number(env, argv[0], &sigma)
        ||  !enif_get_bool(env, argv[1], &boundary_conditions)
        ||  !enif_get_bool(env, argv[2], &is_gaussian)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.blur(sigma, boundary_conditions, is_gaussian);

        return CIMG_GROW;
    }

    CIMG_CMD(mirror) {
        char axis[2];

        if (argc != 1
        ||  !enif_get_atom(env, argv[0], axis, 2, ERL_NIF_LATIN1)
        ||  (axis[0] != 'x' && axis[0] != 'y')) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.mirror(axis[0]);

        return CIMG_GROW;
    }

    CIMG_CMD(transpose) {
        if (argc != 0) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.transpose();

        return CIMG_GROW;
    }

    CIMG_CMD(resize) {
        int width, height;
        int align;
        int filling;

        if (argc != 4
        ||  !enif_get_int(env, argv[0], &width)
        ||  !enif_get_int(env, argv[1], &height)
        ||  !enif_get_int(env, argv[2], &align)
        ||  !enif_get_int(env, argv[3], &filling)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        if (align == 0) {
            img.resize(width, height, -100, -100, 3);
            return CIMG_GROW;
        }
        else if (align == 1 || align == 2) {
            CImgT resized(width, height, img.depth(), img.spectrum(), filling);

            double ratio_w = (double)width/img.width();
            double ratio_h = (double)height/img.height();

            if (ratio_w <= ratio_h) {
                // there is a gap in the vertical direction.
                CImgT tmp_img(img.get_resize(width, ratio_w*img.height(), -100, -100, 3));

                resized.draw_image(0, (align == 1) ? 0 : (height-tmp_img.height()), tmp_img);
            }
            else {
                // there is a gap in the horizontal direction.
                CImgT tmp_img(img.get_resize(ratio_h*img.width(), height, -100, -100, 3));

                resized.draw_image((align == 1) ? 0 : (width-tmp_img.width()), 0, tmp_img);
            }

            resized.move_to(img);
            return CIMG_GROW;
        }
        else {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }
    }

    /**********************************************************************}}}*/
    /* GROW: CImg graphics command implementation                             */
    /**********************************************************************{{{*/
    CIMG_CMD(set) {
        unsigned int x, y, z, c;
        unsigned char val;

        if (argc != 5
        ||  !enif_get_value(env, argv[0], &val)
        ||  !enif_get_uint(env, argv[1], &x)
        ||  !enif_get_uint(env, argv[2], &y)
        ||  !enif_get_uint(env, argv[3], &z)
        ||  !enif_get_uint(env, argv[4], &c)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img(x, y, z, c) = val;

        return CIMG_GROW;
    }

    CIMG_CMD(draw_line) {
        int ix1, iy1, ix2, iy2;
        unsigned char color[3];
        unsigned int thick;
        double opacity;
        unsigned int pattern;

        if (argc != 8
        ||  !enif_get_int(env, argv[0], &ix1)
        ||  !enif_get_int(env, argv[1], &iy1)
        ||  !enif_get_int(env, argv[2], &ix2)
        ||  !enif_get_int(env, argv[3], &iy2)
        ||  !enif_get_color(env, argv[4], color)
        ||  !enif_get_uint(env, argv[5], &thick)
        ||  !enif_get_number(env, argv[6], &opacity)
        ||  !enif_get_uint(env, argv[7], &pattern)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        if (thick <= 2) {
            img.draw_line(ix1, iy1, ix2, iy2, color, opacity, pattern);
        }
        else {
            // Convert line (p1, p2) to polygon (pa, pb, pc, pd)
            const double x_diff = (ix1 - ix2);
            const double y_diff = (iy1 - iy2);
            const double w_diff = thick / 2.0;

            // Triangle between pa and p1: x_adj^2 + y_adj^2 = w_diff^2
            // Triangle between p1 and p2: x_diff^2 + y_diff^2 = length^2
            // Similar triangles: y_adj / x_diff = x_adj / y_diff = w_diff / length
            // -> y_adj / x_diff = w_diff / sqrt(x_diff^2 + y_diff^2)
            const int x_adj = y_diff * w_diff / std::sqrt(std::pow(x_diff, 2) + std::pow(y_diff, 2));
            const int y_adj = x_diff * w_diff / std::sqrt(std::pow(x_diff, 2) + std::pow(y_diff, 2));

            // Points are listed in clockwise order, starting from top-left
            cimg_library::CImg<int> points(4, 2);
            points(0, 0) = ix1 - x_adj;
            points(0, 1) = iy1 + y_adj;
            points(1, 0) = ix1 + x_adj;
            points(1, 1) = iy1 - y_adj;
            points(2, 0) = ix2 + x_adj;
            points(2, 1) = iy2 - y_adj;
            points(3, 0) = ix2 - x_adj;
            points(3, 1) = iy2 + y_adj;

            img.draw_polygon(points, color);
        }

        return CIMG_GROW;
    }

    CIMG_CMD(draw_line_ratio) {
        double x1, y1, x2, y2;
        unsigned char color[3];
        unsigned int thick;
        double opacity;
        unsigned int pattern;

        if (argc != 8
        ||  !enif_get_double(env, argv[0], &x1)
        ||  !enif_get_double(env, argv[1], &y1)
        ||  !enif_get_double(env, argv[2], &x2)
        ||  !enif_get_double(env, argv[3], &y2)
        ||  !enif_get_color(env, argv[4], color)
        ||  !enif_get_uint(env, argv[5], &thick)
        ||  !enif_get_number(env, argv[6], &opacity)
        ||  !enif_get_uint(env, argv[7], &pattern)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        int width  = img.width();
        int height = img.height();

        int ix1 = x1*width;
        int iy1 = y1*height;
        int ix2 = x2*width;
        int iy2 = y2*height;

        if (thick <= 2) {
            img.draw_line(ix1, iy1, ix2, iy2, color, opacity, pattern);
        }
        else {
            // Convert line (p1, p2) to polygon (pa, pb, pc, pd)
            const double x_diff = (ix1 - ix2);
            const double y_diff = (iy1 - iy2);
            const double w_diff = thick / 2.0;

            // Triangle between pa and p1: x_adj^2 + y_adj^2 = w_diff^2
            // Triangle between p1 and p2: x_diff^2 + y_diff^2 = length^2
            // Similar triangles: y_adj / x_diff = x_adj / y_diff = w_diff / length
            // -> y_adj / x_diff = w_diff / sqrt(x_diff^2 + y_diff^2)
            const int x_adj = y_diff * w_diff / std::sqrt(std::pow(x_diff, 2) + std::pow(y_diff, 2));
            const int y_adj = x_diff * w_diff / std::sqrt(std::pow(x_diff, 2) + std::pow(y_diff, 2));

            // Points are listed in clockwise order, starting from top-left
            cimg_library::CImg<int> points(4, 2);
            points(0, 0) = ix1 - x_adj;
            points(0, 1) = iy1 + y_adj;
            points(1, 0) = ix1 + x_adj;
            points(1, 1) = iy1 - y_adj;
            points(2, 0) = ix2 + x_adj;
            points(2, 1) = iy2 - y_adj;
            points(3, 0) = ix2 - x_adj;
            points(3, 1) = iy2 + y_adj;

            img.draw_polygon(points, color);
        }

        return CIMG_GROW;
    }

    CIMG_CMD(draw_circle) {
        int x0;
        int y0;
        int radius;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (argc != 6
        ||  !enif_get_int(env, argv[0], &x0)
        ||  !enif_get_int(env, argv[1], &y0)
        ||  !enif_get_int(env, argv[2], &radius)
        ||  !enif_get_color(env, argv[3], color)
        ||  !enif_get_number(env, argv[4], &opacity)
        ||  !enif_get_uint(env, argv[5], &pattern)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.draw_circle(x0, y0, radius, color, opacity, pattern);

        return CIMG_GROW;
    }

    CIMG_CMD(fill_circle) {
        int x0;
        int y0;
        int radius;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (argc != 6
        ||  !enif_get_int(env, argv[0], &x0)
        ||  !enif_get_int(env, argv[1], &y0)
        ||  !enif_get_int(env, argv[2], &radius)
        ||  !enif_get_color(env, argv[3], color)
        ||  !enif_get_number(env, argv[4], &opacity)
        ||  !enif_get_uint(env, argv[5], &pattern)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.draw_circle(x0, y0, radius, color, opacity);
        if (pattern != 0) {
            img.draw_circle(x0, y0, radius, color, opacity, pattern);
        }

        return CIMG_GROW;
    }

    CIMG_CMD(draw_rectangle) {
        int  x0, y0, x1, y1;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (argc != 7
        ||  !enif_get_int(env, argv[0], &x0)
        ||  !enif_get_int(env, argv[1], &y0)
        ||  !enif_get_int(env, argv[2], &x1)
        ||  !enif_get_int(env, argv[3], &y1)
        ||  !enif_get_color(env, argv[4], color)
        ||  !enif_get_number(env, argv[5], &opacity)
        ||  !enif_get_uint(env, argv[6], &pattern)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.draw_rectangle(x0, y0, x1, y1, color, opacity, pattern);

        return CIMG_GROW;
    }

    CIMG_CMD(draw_rectangle_ratio) {
        double x0, y0, x1, y1;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (argc != 7
        ||  !enif_get_double(env, argv[0], &x0)
        ||  !enif_get_double(env, argv[1], &y0)
        ||  !enif_get_double(env, argv[2], &x1)
        ||  !enif_get_double(env, argv[3], &y1)
        ||  !enif_get_color(env, argv[4], color)
        ||  !enif_get_number(env, argv[5], &opacity)
        ||  !enif_get_uint(env, argv[6], &pattern)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        int width  = img.width();
        int height = img.height();

        int ix0 = x0*width;
        int iy0 = y0*height;
        int ix1 = x1*width;
        int iy1 = y1*height;

        img.draw_rectangle(ix0, iy0, ix1, iy1, color, opacity, pattern);

        return CIMG_GROW;
    }

    CIMG_CMD(fill_rectangle) {
        int  x0, y0, x1, y1;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (argc != 7
        ||  !enif_get_int(env, argv[0], &x0)
        ||  !enif_get_int(env, argv[1], &y0)
        ||  !enif_get_int(env, argv[2], &x1)
        ||  !enif_get_int(env, argv[3], &y1)
        ||  !enif_get_color(env, argv[4], color)
        ||  !enif_get_number(env, argv[5], &opacity)
        ||  !enif_get_uint(env, argv[6], &pattern)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.draw_rectangle(x0, y0, x1, y1, color, opacity);
        if (pattern != 0) {
            img.draw_rectangle(x0, y0, x1, y1, color, 1.0, pattern);
        }

        return CIMG_GROW;
    }

    CIMG_CMD(fill_rectangle_ratio) {
        double x0, y0, x1, y1;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (argc != 7
        ||  !enif_get_double(env, argv[0], &x0)
        ||  !enif_get_double(env, argv[1], &y0)
        ||  !enif_get_double(env, argv[2], &x1)
        ||  !enif_get_double(env, argv[3], &y1)
        ||  !enif_get_color(env, argv[4], color)
        ||  !enif_get_number(env, argv[5], &opacity)
        ||  !enif_get_uint(env, argv[6], &pattern)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        int width  = img.width();
        int height = img.height();

        int ix0 = x0*width;
        int iy0 = y0*height;
        int ix1 = x1*width;
        int iy1 = y1*height;

        img.draw_rectangle(ix0, iy0, ix1, iy1, color, opacity);
        if (pattern != 0) {
            img.draw_rectangle(ix0, iy0, ix1, iy1, color, 1.0, pattern);
        }

        return CIMG_GROW;
    }

    CIMG_CMD(draw_triangle) {
        int  x0, y0, x1, y1, x2, y2;
        unsigned char color[3];
        double opacity;
        unsigned int pattern;

        if (argc != 9
        ||  !enif_get_int(env, argv[0], &x0)
        ||  !enif_get_int(env, argv[1], &y0)
        ||  !enif_get_int(env, argv[2], &x1)
        ||  !enif_get_int(env, argv[3], &y1)
        ||  !enif_get_int(env, argv[4], &x2)
        ||  !enif_get_int(env, argv[5], &y2)
        ||  !enif_get_color(env, argv[6], color)
        ||  !enif_get_number(env, argv[7], &opacity)
        ||  !enif_get_uint(env, argv[8], &pattern)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.draw_triangle(x0, y0, x1, y1, x2, y2, color, opacity, pattern);

        return CIMG_GROW;
    }

    CIMG_CMD(draw_triangle_filled) {
        int  x0, y0, x1, y1, x2, y2;
        unsigned char color[3];
        double opacity;

        if (argc != 8
        ||  !enif_get_int(env, argv[0], &x0)
        ||  !enif_get_int(env, argv[1], &y0)
        ||  !enif_get_int(env, argv[2], &x1)
        ||  !enif_get_int(env, argv[3], &y1)
        ||  !enif_get_int(env, argv[4], &x2)
        ||  !enif_get_int(env, argv[5], &y2)
        ||  !enif_get_color(env, argv[6], color)
        ||  !enif_get_number(env, argv[7], &opacity)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.draw_triangle(x0, y0, x1, y1, x2, y2, color, opacity);

        return CIMG_GROW;
    }

    CIMG_CMD(draw_graph) {
        CImgT* data;
        unsigned char color[3];
        double        opacity;
        unsigned int plot_type;
        int           vertex_type;
        double        ymin;
        double        ymax;
        unsigned int pattern;

        if (argc != 8
        ||  !enif_get_image(env, argv[0], &data)
        ||  !enif_get_color(env, argv[1], color)
        ||  !enif_get_number(env, argv[2], &opacity)
        ||  !enif_get_uint(env, argv[3], &plot_type)
        ||  !enif_get_int(env, argv[4], &vertex_type)
        ||  !enif_get_number(env, argv[5], &ymin)
        ||  !enif_get_number(env, argv[6], &ymax)
        ||  !enif_get_uint(env, argv[7], &pattern)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.draw_graph(*data, color, opacity, plot_type, vertex_type, ymin, ymax, pattern);

        return CIMG_GROW;
    }

    CIMG_CMD(draw_morph) {
        int cx, cy, cz;
        if (argc != 4
        ||  !enif_is_list(env, argv[0])
        ||  !enif_get_int(env, argv[1], &cx)
        ||  !enif_get_int(env, argv[2], &cy)
        ||  !enif_get_int(env, argv[3], &cz)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        CImgT src(img);

        ERL_NIF_TERM list = argv[0], head;
        while (enif_get_list_cell(env, list, &head, &list)) {
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

            if (img.containsXYZC(q[0], q[1], q[2]) && src.containsXYZC(p[0], p[1], p[2])) {
                cimg_forC(src, c) {
                    img(q[0], q[1], q[2], c) = src(p[0], p[1], p[2], c);
                }
            }
        }

        return CIMG_GROW;
    }

    CIMG_CMD(draw_text) {
        int x, y;
        std::string text;
        const unsigned char *fg_color, *bg_color;
        double opacity;
        unsigned int font_height;

        if (argc != 7
        ||  !enif_get_int(env, argv[0], &x)
        ||  !enif_get_int(env, argv[1], &y)
        ||  !enif_get_str(env, argv[2], &text)
        ||  !enif_get_color_name(env, argv[3], &fg_color)
        ||  !enif_get_color_name(env, argv[4], &bg_color)
        ||  !enif_get_double(env, argv[5], &opacity)
        ||  !enif_get_uint(env, argv[6], &font_height)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.draw_text(x, y, text.c_str(), fg_color, bg_color, opacity, font_height);

        return CIMG_GROW;
    }

    /**********************************************************************}}}*/
    /* CROP: CImg output command implementation                               */
    /**********************************************************************{{{*/
    CIMG_CMD(get_image) {
        if (argc != 0) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        CImgT* res_img = new CImgT(img);
        res = enif_make_image(env, res_img);

        return CIMG_CROP;
    }

    CIMG_CMD(get_shape) {
        if (argc != 0) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        res = enif_make_tuple4(env,
            enif_make_int(env, img.width()),
            enif_make_int(env, img.height()),
            enif_make_int(env, img.depth()),
            enif_make_int(env, img.spectrum()));

        return CIMG_CROP;
    }

    CIMG_CMD(get_size) {
        if (argc != 0) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        res = enif_make_ulong(env, img.size());

        return CIMG_CROP;
    }

    CIMG_CMD(save) {
        std::string fname;

        if (argc != 1
        ||  !enif_get_str(env, argv[0], &fname)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.save(fname.c_str());
        res = enif_make_ok(env);

        return CIMG_CROP;
    }

    CIMG_CMD(to_image) {
        char format[5];

        if (argc != 1
        ||  !enif_get_atom(env, argv[0], format, sizeof(format), ERL_NIF_LATIN1)
        ||  (std::strcmp(format, "jpeg") != 0 && std::strcmp(format, "png") != 0)) {
            res = enif_make_badarg(env);
        }

        auto mem = img.save_to_memory(format);
        if (mem.size() == 0) {
            res = enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't convert empty image", ERL_NIF_LATIN1));
            return CIMG_ERROR;
        }

        ERL_NIF_TERM binary;
        unsigned char* buff = enif_make_new_binary(env, mem.size(), &binary);
        if (buff == NULL) {
            res = enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't alloc binary", ERL_NIF_LATIN1));
            return CIMG_ERROR;
        }

        std::memcpy(buff, mem.data(), mem.size());
        res = enif_make_tuple2(env, enif_make_ok(env), binary);

        return CIMG_CROP;
    }

    CIMG_CMD(to_bin) {
        std::string dtype;
        char conv_op[8];
        const ERL_NIF_TERM* conv_prms;
        int conv_prms_count;
        bool nchw;    // to transpose NCHW
        bool bgr;     // to convert RGB to BGR

        if (argc != 5
        ||  !enif_get_str(env, argv[0], &dtype)
        ||  !enif_get_atom(env, argv[1], conv_op, sizeof(conv_op), ERL_NIF_LATIN1)
        ||  !enif_get_tuple(env, argv[2], &conv_prms_count, &conv_prms)
        ||  !enif_get_bool(env, argv[3], &nchw)
        ||  !enif_get_bool(env, argv[4], &bgr)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        // select BGR convertion
        int color[4] = {0,1,2,3};
        if (bgr && img.spectrum() >= 3) {
            int tmp_c = color[0]; color[0] = color[2]; color[2] = tmp_c;
        }

        ERL_NIF_TERM binary;
        if (dtype == "<f4") {
            /* setup normalization converter **********************************/
            double a[4], b[4];
            if (strcmp(conv_op, "gauss") == 0 && conv_prms_count == 3) {
                for (int i = 0; i < conv_prms_count; i++) {
                    int stat_prms_count;
                    const ERL_NIF_TERM* stat_prms;
                    double mu, sigma;
                    if (!enif_get_tuple(env, conv_prms[i], &stat_prms_count, &stat_prms)
                    ||  stat_prms_count != 2
                    ||  !enif_get_double(env, stat_prms[0], &mu)
                    ||  !enif_get_double(env, stat_prms[1], &sigma)) {
                        res = enif_make_badarg(env);
                        return CIMG_ERROR;
                    }

                    a[color[i]] = 1.0/sigma;
                    b[color[i]] = -mu/sigma;
                }
                a[3] = 1.0/255.0;
                b[3] = 0.0;
            }
            else if (strcmp(conv_op, "range") == 0 && conv_prms_count == 2) {
                double lo, hi;
                if (!enif_get_double(env, conv_prms[0], &lo)
                ||  !enif_get_double(env, conv_prms[1], &hi)) {
                    res = enif_make_badarg(env);
                    return CIMG_ERROR;
                }

                for (int i = 0; i < 3; i++) {
                    a[color[i]] = (hi - lo)/255.0;
                    b[color[i]] = lo;
                }
                a[3] = 1.0/255.0;
                b[3] = 0.0;
            }
            else {
                res = enif_make_badarg(env);
                return CIMG_ERROR;
            }

            auto convert = [a, b](int c, int x) {
                return static_cast<float>(a[c]*x + b[c]);
            };
            /* ****************************************************************/


            float* buff = reinterpret_cast<float*>(enif_make_new_binary(env, 4*img.size(), &binary));
            if (buff == NULL) {
                res = enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't alloc binary", ERL_NIF_LATIN1));
                return CIMG_ERROR;
            }

            if (nchw) {
                cimg_forC(img, c) cimg_forXY(img, x, y) {
                    *buff++ = convert(c, img(x, y, color[c]));
                }
            }
            else {
                cimg_forXY(img, x, y) cimg_forC(img, c) {
                    *buff++ = convert(c, img(x, y, color[c]));
                }
            }
        }
        else {
            unsigned char* buff = enif_make_new_binary(env, img.size(), &binary);
            if (buff == NULL) {
                res = enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, "can't alloc binary", ERL_NIF_LATIN1));
                return CIMG_ERROR;
            }

            if (nchw) {
                cimg_forC(img, c) cimg_forXY(img, x, y) {
                    *buff++ = img(x, y,  color[c]);
                }
            }
            else {
                cimg_forXY(img, x, y) cimg_forC(img, c) {
                    *buff++ = img(x, y,  color[c]);
                }
            }
        }

        ERL_NIF_TERM shape;
        if (nchw) {
            shape = enif_make_tuple3(env,
                enif_make_int(env, img.spectrum()),
                enif_make_int(env, img.height()),
                enif_make_int(env, img.width()));
        }
        else {
            shape = enif_make_tuple3(env,
                enif_make_int(env, img.height()),
                enif_make_int(env, img.width()),
                enif_make_int(env, img.spectrum()));
        }

        res = enif_make_tuple3(env, enif_make_ok(env), shape, binary);

        return CIMG_CROP;
    }

    CIMG_CMD(get) {
        unsigned int x, y, z, c;
        unsigned char val;

        if (argc != 4
        ||  !enif_get_uint(env, argv[0], &x)
        ||  !enif_get_uint(env, argv[1], &y)
        ||  !enif_get_uint(env, argv[2], &z)
        ||  !enif_get_uint(env, argv[3], &c)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        val = img(x, y, z, c);

        res = enif_make_value(env, val);

        return CIMG_CROP;
    }

    CIMG_CMD(get_crop) {
        int x0, y0, z0, c0;
        int x1, y1, z1, c1;
        unsigned int boundary_conditions;

        if (argc != 9
        ||  !enif_get_int(env, argv[0], &x0)
        ||  !enif_get_int(env, argv[1], &y0)
        ||  !enif_get_int(env, argv[2], &z0)
        ||  !enif_get_int(env, argv[3], &c0)
        ||  !enif_get_int(env, argv[4], &x1)
        ||  !enif_get_int(env, argv[5], &y1)
        ||  !enif_get_int(env, argv[6], &z1)
        ||  !enif_get_int(env, argv[7], &c1)
        ||  !enif_get_uint(env, argv[8], &boundary_conditions)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        CImgT* crop;
        try {
            crop = new CImgT(img.get_crop(x0, y0, z0, c0, x1, y1, z1, c1, boundary_conditions));
        }
        catch (CImgException& e) {
            res = enif_make_tuple2(env, enif_make_error(env), enif_make_string(env, e.what(), ERL_NIF_LATIN1));
            return CIMG_ERROR;
        }

        res = enif_make_image(env, crop);

        return CIMG_CROP;
    }

    CIMG_CMD(display) {
    #if cimg_display != 0
        if (argc != 0) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.display();

        res = enif_make_ok(env);
    #else
        res = enif_make_error(env);
    #endif
        return CIMG_CROP;
    }

    CIMG_CMD(display_on) {
    #if cimg_display != 0
        CImgDisplay* disp;

        if (argc != 1
        ||  !NifCImgDisplay::enif_get_display(env, argv[0], &disp)) {
            res = enif_make_badarg(env);
            return CIMG_ERROR;
        }

        img.display(*disp);

        res = enif_make_ok(env);
    #else
        res = enif_make_error(env);
    #endif
        return CIMG_CROP;
    }
}

#endif
/*** cimg_cmd.h ***********************************************************}}}*/