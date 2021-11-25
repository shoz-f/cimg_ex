defmodule CImg.NIF do
  @moduledoc """
  NIFs entries.
  """
  # loading NIF library
  @on_load :load_nif
  def load_nif do
    nif_file = Application.app_dir(:cimg, "priv/cimg_nif")
    :erlang.load_nif(nif_file, 0)
  end

  # stub implementations for NIFs (fallback)
  def cimg_create(_x, _y, _z, _c, _v),
    do: raise("NIF cimg_create/5 not implemented")
  def cimg_from_bin(_dtype, _x, _y, _z, _c, _f4),
    do: raise("NIF cimg_from_bin/6 not implemented")
  def cimg_dup(_cimgu8),
    do: raise("NIF cimg_create/1 not implemented")
  def cimg_clear(_cimgu8),
    do: raise("NIF cimg_clear/1 not implemented")
  def cimg_load(_s),
    do: raise("NIF cimg_load/1 not implemented")
  def cimg_save(_c, _s),
    do: raise("NIF cimg_save/2 not implemented")
  def cimg_resize(_c, _x, _y),
    do: raise("NIF cimg_resize/3 not implemented")
  def cimg_get_resize(_c, _x, _y),
    do: raise("NIF cimg_resize/3 not implemented")
  def cimg_get_packed(_c, _x, _y, _v),
    do: raise("NIF cimg_packed/4 not implemented")
  def cimg_mirror(_c, _axis),
    do: raise("NIF cimg_mirror/2 not implemented")
  def cimg_get_gray(_c, _pn),
    do: raise("NIF cimg_get_gray/2 not implemented")
  def cimg_blur(_c, _s, _b, _g),
    do: raise("NIF cimg_blur/4 not implemented")
  def cimg_get_crop(_c, _x0, _y0, _z0, _c0, _x1, _y1, _z1, _c1, _b),
    do: raise("NIF cimg_get_crop/10 not implemented")
  def cimg_fill(_c, _val),
    do: raise("NIF cimg_fill/2 not implemented")
  def cimg_draw_graph(_c, _d, _color, _o, _p, _v, _ymin, _ymax, _pat),
    do: raise("NIF cimg_draw_graph/9 not implemented")
  def cimg_to_bin(_c, _dtype, _lo, _hi, _nchw, _bgr),
    do: raise("NIF cimg_to_bin/6 not implemented")
  def cimg_draw_box(_c, _x0, _y0, _x1, _y1, _rgb),
    do: raise("NIF cimg_draw_box/6 not implemented")
  def cimg_display(_cimgu8, _disp),
    do: raise("NIF cimg_display/2 not implemented")
  def cimg_set(_val, _cimgu8, _x, _y, _z, _c),
    do: raise("NIF cimg_set/6 not implemented")
  def cimg_get(_cimgu8, _x, _y, _z, _c),
    do: raise("NIF cimg_get/5 not implemented")
  def cimg_assign(_cimgu8, _cimgu8_src),
    do: raise("NIF cimg_assign/2 not implemented")
  def cimg_draw_circle(_cimgu8, _x0, _y0, _radius, _color, _opacity),
    do: raise("NIF cimg_draw_circle/6 not implemented")
  def cimg_draw_circle(_cimgu8, _x0, _y0, _radius, _color, _opacity, _pattern),
    do: raise("NIF cimg_draw_circle/7 not implemented")
  def cimg_draw_rect(_cimgu8, _x0, _y0, _x1, _y1, _color, _opacity, _pattern),
    do: raise("NIF cimg_draw_rect/8 not implemented")
  def cimg_shape(_cimgu8),
    do: raise("NIF cimg_shape/1 not implemented")
  def cimg_size(_cimgu8),
    do: raise("NIF cimg_size/1 not implemented")
  def cimg_transfer(_cimgu8, _cimgu8_src, _mapping, _cx, _cy, _cz),
    do: raise("NIF cimg_transfer/6 not implemented")
  def cimg_map(_c, _lut, _boundary),
    do: raise("NIF cimg_map/3 not implemented")
  def cimg_threshold(_c, _val, _soft, _strict),
    do: raise("NIF cimg_threshold/4 not implemented")


  def cimgdisplay_u8(_cimgu8, _title, _normalization, _is_fullscreen, _is_close),
    do: raise("NIF cimgdisplay_u8/5 not implemented")
  def cimgdisplay_wait(_disp),
    do: raise("NIF cimgdisplay_wait/1 not implemented")
  def cimgdisplay_wait(_disp, _milliseconds),
    do: raise("NIF cimgdisplay_wait/2 not implemented")
  def cimgdisplay_is_closed(_disp),
    do: raise("NIF cimgdisplay_is_closed/1 not implemented")
  def cimgdisplay_button(_disp),
    do: raise("NIF cimgdisplay_button/1 not implemented")
  def cimgdisplay_mouse_y(_disp),
    do: raise("NIF cimgdisplay_mouse_y/1 not implemented")
end
