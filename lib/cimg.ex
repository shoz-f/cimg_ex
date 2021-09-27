defmodule CImg do
  @moduledoc """
  CImg image processing extention.
  """
  alias __MODULE__

  # image object
  defstruct handle: nil, shape: {}

  @doc """
  load the image file and create new image object.
  """
  def create(fname) do
    with {:ok, h, [shape]} <- CImgNIF.cimg_load(fname),
      do: %CImg{handle: h, shape: shape}
  end
  
  def create(x, y, z, c, val) when is_integer(val) do
    with {:ok, h, [shape]} <- CImgNIF.cimg_create(x, y, z, c, val),
      do: %CImg{handle: h, shape: shape}
  end

  @doc "save image object to the file"
  defdelegate save(cimg, fname),
    to: CImgNIF, as: :cimg_save

  @doc "resize the image object"
  def resize(cimg, [x, y]), do: CImgNIF.cimg_resize(cimg, x, y)

  def get_resize(cimg, [x, y]) do
    with {:ok, resize, [shape]} <- CImgNIF.cimg_get_resize(cimg, x, y),
      do: %CImg{handle: resize, shape: shape}
  end

  defdelegate blur(cimg, sigma, boundary_conditions \\ true, is_gaussian \\ true),
    to: CImgNIF, as: :cimg_blur

  @doc "mirroring the image object on the axis"
  def mirror(cimg, axis) when axis in [:x, :y] do
    CImgNIF.cimg_mirror(cimg, axis)
  end

  @doc """
  create new gray image object from the image object
  """
  def get_gray(cimg, opt_pn \\ 0) do
    with {:ok, gray, [shape]} <- CImgNIF.cimg_get_gray(cimg, opt_pn),
      do: %CImg{handle: gray, shape: shape}
  end

  def get_crop(cimg, x0, y0, z0, c0, x1, y1, z1, c1, boundary_conditions \\ 0) do
    with {:ok, crop, [shape]} <- CImgNIF.cimg_get_crop(cimg, x0, y0, z0, c0, x1, y1, z1, c1, boundary_conditions),
      do: %CImg{handle: crop, shape: shape}
  end

  @doc "get the flat binary from the image object"
  def to_flatbin(cimg, nchw \\ false) do
    with {:ok, bin} <- CImgNIF.cimg_get_flatbin(cimg, nchw),
      do: %{descr: "<u1", fortran_order: false, shape: shape(cimg), data: bin}
  end

  @doc "get the normalized flat binary from the image object"
  def to_flatnorm(cimg, nchw \\ false) do
    with {:ok, bin} <- CImgNIF.cimg_get_flatnorm(cimg, nchw),
      do: %{descr: "<f4", fortran_order: false, shape: shape(cimg), data: bin}
  end
  
  @doc "create new image object from float binaries."
  def create_from_f4bin(x, y, z, c, f4) when is_binary(f4) do
    with {:ok, h, [shape]} <- CImgNIF.cimg_from_f4bin(x, y, z, c, f4),
      do: %CImg{handle: h, shape: shape}
  end

  @doc """
  draw the colored box on the image object
  """
  def draw_box(cimg, x0, y0, x1, y1, rgb) do
    CImgNIF.cimg_draw_box(cimg, x0, y0, x1, y1, rgb)
  end
  
  defdelegate display(cimg, disp),
    to: CImgNIF, as: :cimg_display
  defdelegate fill(cimg, val),
    to: CImgNIF, as: :cimg_fill
  defdelegate draw_graph(cimg, data, color, opacity \\ 1.0, plot_type \\ 1, vertex_type \\ 1, ymin \\ 0.0, ymax \\ 0.0, pattern \\ 0xFFFFFFFF),
    to: CImgNIF, as: :cimg_draw_graph
  defdelegate set(val, cimg, x, y \\ 0, z \\ 0, c \\ 0),
    to: CImgNIF, as: :cimg_set
  defdelegate get(cimg, x, y \\ 0, z \\ 0, c \\ 0),
    to: CImgNIF, as: :cimg_get
  defdelegate assign(cimg, cimg_src),
    to: CImgNIF, as: :cimg_assign
  defdelegate draw_circle(cimg, x0, y0, radius, color, opacity \\ 1.0),
    to: CImgNIF, as: :cimg_draw_circle
  defdelegate draw_circle(cimg, x0, y0, radius, color, opacity, pattern),
    to: CImgNIF, as: :cimg_draw_circle
  defdelegate shape(cimg),
    to: CImgNIF, as: :cimg_shape
  defdelegate transfer(cimg, cimg_src, mapping, cx \\ 0, cy \\ 0, cz \\ 0),
    to: CImgNIF, as: :cimg_transfer
end

defmodule CImgMap do
  alias __MODULE__

  # image object
  defstruct handle: nil, shape: {}

  @doc "load the image file and create new image object."
  def create(x, y, z, c, list) when is_list(list) do
    with {:ok, h, [shape]} <- CImgNIF.cimgmap_create(x, y, z, c, list),
      do: %CImgMap{handle: h, shape: shape}
  end

  defdelegate set(val, cimgmap, x, y \\ 0, z \\ 0, c \\ 0),
    to: CImgNIF, as: :cimgmap_set
  defdelegate get(cimgmap, x, y \\ 0, z \\ 0, c \\ 0),
    to: CImgNIF, as: :cimgmap_get
end

defmodule CImgDisplay do
  alias __MODULE__

  defstruct handle: nil

  def create(%CImg{} = cimg, title \\ "", normalization \\ 3, is_fullscreen \\ false, is_closed \\ false) do
    with {:ok, h, _} <- CImgNIF.cimgdisplay_u8(cimg, title, normalization, is_fullscreen, is_closed),
      do: %CImgDisplay{handle: h}
  end
  
  defdelegate wait(cimgdisplay),
    to: CImgNIF, as: :cimgdisplay_wait
  defdelegate wait(cimgdisplay, milliseconds),
    to: CImgNIF, as: :cimgdisplay_wait
  defdelegate is_closed(cimgdisplay),
    to: CImgNIF, as: :cimgdisplay_is_closed
  defdelegate button(cimgdisplay),
    to: CImgNIF, as: :cimgdisplay_button
  defdelegate mouse_y(cimgdisplay),
    to: CImgNIF, as: :cimgdisplay_mouse_y
end


defmodule CImgNIF do
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
  def cimg_create(_cimgu8),
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
  def cimg_get_flatbin(_c, _nchw),
    do: raise("NIF cimg_get_flatbin/2 not implemented")
  def cimg_get_flatnorm(_c, _nchw),
    do: raise("NIF cimg_get_flatnorm/2 not implemented")
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
  def cimg_shape(_cimgu8),
    do: raise("NIF cimg_shape/1 not implemented")
  def cimg_transfer(_cimgu8, _cimgu8_src, _mapping, _cx, _cy, _cz),
    do: raise("NIF cimg_transfer/6 not implemented")
  def cimg_from_f4bin(_x, _y, _z, _c, _f4),
    do: raise("NIF cimg_create/5 not implemented")


  def cimgmap_create(_x, _y, _z, _c, _list),
    do: raise("NIF cimgmap_create/5 not implemented")
  def cimgmap_set(_val, _cimgu8, _x, _y, _z, _c),
    do: raise("NIF cimgmap_set/6 not implemented")
  def cimgmap_get(_cimgu8, _x, _y, _z, _c),
    do: raise("NIF cimgmap_get/5 not implemented")

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
