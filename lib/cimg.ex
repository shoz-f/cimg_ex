defmodule CImg do
  @moduledoc """
  Light-weight image processing extention.
  """
  alias __MODULE__
  alias CImg.NIF

  # image object
  defstruct handle: nil, shape: {}

  @doc """
  create image filled val
  """
  def create(x, y, z, c, val) when is_integer(val) do
    with {:ok, h, [shape]} <- NIF.cimg_create(x, y, z, c, val),
      do: %CImg{handle: h, shape: shape}
  end
  
  @doc "create new image object from byte binaries."
  def create_from_bin(bin, x, y, z, c, dtype) when is_binary(bin) do
    with {:ok, h, [shape]} <- NIF.cimg_from_bin(bin, x, y, z, c, dtype),
      do: %CImg{handle: h, shape: shape}
  end

  @doc """
  load the image file and create new image object.
  """
  def load(fname) do
    with {:ok, h, [shape]} <- NIF.cimg_load(fname),
      do: %CImg{handle: h, shape: shape}
  end

  @doc """
  duplicate image
  """
  def dup(cimg) do
    with {:ok, h, [shape]} <- NIF.cimg_dup(cimg),
      do: %CImg{handle: h, shape: shape}
  end

  @doc "save image object to the file"
  defdelegate save(cimg, fname),
    to: NIF, as: :cimg_save

  @doc "resize the image object"
  def resize(cimg, {x, y}), do: NIF.cimg_resize(cimg, x, y)

  @doc "get a image object resized [x, y]"
  def get_resize(cimg, {x, y}) do
    with {:ok, resize, [shape]} <- NIF.cimg_get_resize(cimg, x, y),
      do: %CImg{handle: resize, shape: shape}
  end
  
  @doc "get a image object packed into the box[x,y]"
  def get_packed(cimg, {x, y}, fill) do
    with {:ok, packed, [shape]} <- NIF.cimg_get_packed(cimg, x, y, fill),
      do: %CImg{handle: packed, shape: shape}
  end

  defdelegate blur(cimg, sigma, boundary_conditions \\ true, is_gaussian \\ true),
    to: NIF, as: :cimg_blur

  @doc "mirroring the image object on the axis"
  def mirror(cimg, axis) when axis in [:x, :y] do
    NIF.cimg_mirror(cimg, axis)
  end

  @doc """
  create new gray image object from the image object
  """
  def get_gray(cimg, opt_pn \\ 0) do
    with {:ok, gray, [shape]} <- NIF.cimg_get_gray(cimg, opt_pn),
      do: %CImg{handle: gray, shape: shape}
  end

  def get_crop(cimg, x0, y0, z0, c0, x1, y1, z1, c1, boundary_conditions \\ 0) do
    with {:ok, crop, [shape]} <- NIF.cimg_get_crop(cimg, x0, y0, z0, c0, x1, y1, z1, c1, boundary_conditions),
      do: %CImg{handle: crop, shape: shape}
  end

  @doc "get the flat binary from the image object"
  def to_flatbin(cimg, nchw \\ false, bgr \\ false) do
    case {nchw, bgr} do
      {false, false} -> to_flat(cimg, [{:dtype, "<u1"}])
      {false, true } -> to_flat(cimg, [{:dtype, "<u1"}, :bgr])
      {true,  false} -> to_flat(cimg, [{:dtype, "<u1"}, :nchw])
      {true,  true } -> to_flat(cimg, [{:dtype, "<u1"}, :nchw, :bgr])
    end
  end

  @doc "get the float32 flat binary from the image object"
  def to_flatf4(cimg, nchw \\ false, bgr \\ false) do
    case {nchw, bgr} do
      {false, false} -> to_flat(cimg, [{:range, {0.0, 255.0}}])
      {false, true } -> to_flat(cimg, [{:range, {0.0, 255.0}}, :bgr])
      {true,  false} -> to_flat(cimg, [{:range, {0.0, 255.0}}, :nchw])
      {true,  true } -> to_flat(cimg, [{:range, {0.0, 255.0}}, :nchw, :bgr])
    end
  end

  @doc "get the normalized float32 flat binary from the image object"
  def to_flatnorm(cimg, nchw \\ false, bgr \\ false) do
    case {nchw, bgr} do
      {false, false} -> to_flat(cimg, [])
      {false, true } -> to_flat(cimg, [:bgr])
      {true,  false} -> to_flat(cimg, [:nchw])
      {true,  true } -> to_flat(cimg, [:nchw, :bgr])
    end
  end
  
  @doc "get the flat binary"
  def to_flat(cimg, opts \\ []) do
    dtype    = Keyword.get(opts, :dtype, "<f4")
    {lo, hi} = Keyword.get(opts, :range, {0.0, 1.0})
    nchw     = :nchw in opts
    bgr      = :bgr  in opts

    with {:ok, bin} <- NIF.cimg_to_bin(cimg, dtype, lo, hi, nchw, bgr) do
      %{descr: dtype, fortran_order: false, shape: {size(cimg)}, data: bin}
    end
  end
  
  @doc "create new image object from float binaries."
  def create_from_f4bin(x, y, z, c, bin) do
    create_from_bin(bin, x, y, z, c, "<f4")
  end

  def map(cimg, lut, boundary \\ 0) do
    with {:ok, h, [shape]} <- NIF.cimg_map(cimg, lut, boundary),
      do: %CImg{handle: h, shape: shape}
  end

  @doc """
  draw the colored box on the image object
  """
  def draw_box(cimg, x0, y0, x1, y1, rgb) do
    NIF.cimg_draw_box(cimg, x0, y0, x1, y1, rgb)
  end
  
  defdelegate display(cimg, disp),
    to: NIF, as: :cimg_display
  defdelegate fill(cimg, val),
    to: NIF, as: :cimg_fill
  defdelegate draw_graph(cimg, data, color, opacity \\ 1.0, plot_type \\ 1, vertex_type \\ 1, ymin \\ 0.0, ymax \\ 0.0, pattern \\ 0xFFFFFFFF),
    to: NIF, as: :cimg_draw_graph
  defdelegate set(val, cimg, x, y \\ 0, z \\ 0, c \\ 0),
    to: NIF, as: :cimg_set
  defdelegate get(cimg, x, y \\ 0, z \\ 0, c \\ 0),
    to: NIF, as: :cimg_get
  defdelegate assign(cimg, cimg_src),
    to: NIF, as: :cimg_assign
  defdelegate draw_circle(cimg, x0, y0, radius, color, opacity \\ 1.0),
    to: NIF, as: :cimg_draw_circle
  defdelegate draw_circle(cimg, x0, y0, radius, color, opacity, pattern),
    to: NIF, as: :cimg_draw_circle
  defdelegate draw_rect(cimg, x0, y0, x1, y1, color, opacity \\ 1.0, pattern \\ 0xFFFFFFFF),
    to: NIF, as: :cimg_draw_rect
  defdelegate shape(cimg),
    to: NIF, as: :cimg_shape
  defdelegate size(cimg),
    to: NIF, as: :cimg_size
  defdelegate transfer(cimg, cimg_src, mapping, cx \\ 0, cy \\ 0, cz \\ 0),
    to: NIF, as: :cimg_transfer
end


defmodule CImgMap do
  alias __MODULE__

  # image object
  defstruct handle: nil, shape: {}

  @doc "load the image file and create new image object."
  def create(x, y, z, c, list) when is_list(list) do
    with {:ok, h, [shape]} <- CImg.NIF.cimgmap_create(x, y, z, c, list),
      do: %CImgMap{handle: h, shape: shape}
  end

  defdelegate set(val, cimgmap, x, y \\ 0, z \\ 0, c \\ 0),
    to: CImg.NIF, as: :cimgmap_set
  defdelegate get(cimgmap, x, y \\ 0, z \\ 0, c \\ 0),
    to: CImg.NIF, as: :cimgmap_get
end


defmodule CImgDisplay do
  alias __MODULE__

  defstruct handle: nil

  def create(%CImg{} = cimg, title \\ "", normalization \\ 3, is_fullscreen \\ false, is_closed \\ false) do
    with {:ok, h, _} <- CImg.NIF.cimgdisplay_u8(cimg, title, normalization, is_fullscreen, is_closed),
      do: %CImgDisplay{handle: h}
  end
  
  defdelegate wait(cimgdisplay),
    to: CImg.NIF, as: :cimgdisplay_wait
  defdelegate wait(cimgdisplay, milliseconds),
    to: CImg.NIF, as: :cimgdisplay_wait
  defdelegate is_closed(cimgdisplay),
    to: CImg.NIF, as: :cimgdisplay_is_closed
  defdelegate button(cimgdisplay),
    to: CImg.NIF, as: :cimgdisplay_button
  defdelegate mouse_y(cimgdisplay),
    to: CImg.NIF, as: :cimgdisplay_mouse_y
end
