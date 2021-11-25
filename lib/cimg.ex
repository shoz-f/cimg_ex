defmodule CImg do
  @moduledoc """
  Light-weight image processing extention.
  """
  alias __MODULE__
  alias CImg.NIF

  # image object
  #   :handle - Erlang resource object pointing to the CImg image.
  #   :shape  - demensions {x, y, z, spectrum} of image.
  defstruct handle: nil, shape: {}

  @doc """
  Create image{x,y,z,c} filled `val`.

  ## Parameters

    * x,y,z,c - image's x-size, y-size, z-size and spectrum.
    * val     - filling value.


  ## Examples

    ```Elixir
    iex> img = CImg.create(200, 100, 1, 3, 127)
    ```
  """
  def create(x, y, z, c, val) when is_integer(val) do
    with {:ok, h, [shape]} <- NIF.cimg_create(x, y, z, c, val),
      do: %CImg{handle: h, shape: shape}
  end

  @doc """
  Create image{x,y,z,c} from byte binary. `create_from_bin` helps you to make
  the image from the serialiezed output tensor of DNN model.

  ## Parameters

    * bin - binary data to have in an image.
    * x,y,z,c - image's x-size, y-size, z-size and spectrum.
    * dtype - data type in the binary. any data types are converted to int8 in the image.
      - "<f4" - 32bit float (available value in range 0.0..1.0)
      - "<u1" - 8bit unsigned integer

  ## Examples

    ```Elixir
    iex> bin = TflInterp.get_output_tensor(__MODULE__, 0)
    iex> img = CImg.create_from_bin(bin, 300, 300, 1, 3, "<f4")
    ```
  """
  def create_from_bin(bin, x, y, z, c, dtype) when is_binary(bin) do
    with {:ok, h, [shape]} <- NIF.cimg_from_bin(bin, x, y, z, c, dtype),
      do: %CImg{handle: h, shape: shape}
  end

  @doc """
  Load a image from file.

  ## Parameters

    * fname - file path of the image. (only jpeg images - xxx.jpg - are available now)

  ## Examples

    ```Elixir
    iex> img = CImg.load("sample.jpg")
    ```
  """
  def load(fname) do
    with {:ok, h, [shape]} <- NIF.cimg_load(fname),
      do: %CImg{handle: h, shape: shape}
  end

  @doc """
  Duplicate the image.

  ## Parameters

    * cimg - image object %CImg{} to duplicate.

  ## Examples

    ```
    iex> img = CImg.dup(original)
    # create new image object `img` have same shape and values of original.
    ```
  """
  def dup(cimg) do
    with {:ok, h, [shape]} <- NIF.cimg_dup(cimg),
      do: %CImg{handle: h, shape: shape}
  end

  @doc """
  Save the image to the file.

  ## Parameters

    * cimg - image object %CImg{} to save.
    * fname - file path for the image. (only jpeg images - xxx.jpg - are available now)

  """
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

  @doc """
  Get serialized binary of the image from top-left to bottom-right.
  `to_flat/2` helps you to make 32bit-float arrary for the input tensors of DNN model.

  ## Parameters

    * cimg - image object.
    * opts - conversion options
      - { :dtype, xx } - convert pixel value to data type.
           available: "<f4"/32bit-float, "<u1"/8bit-unsigned-char
      - { :range, {lo, hi} } - normarilzed range when :dtype is "<f4".
           default range: {0.0, 1.0}
      - :nchw - transform axes NHWC to NCHW.
      - :bgr - convert color RGB -> BGR.

  ## Examples

    ```Elixir
    iex> img = CImg.load("sample.jpg")
    iex> bin1 = CImg.to_flat(img, [{dtype: "<f4"}, {:range, {-1.0, 1.0}}, :nchw])
    # convert pixel value to 32bit-float in range -1.0..1.0 and transform axis to NCHW.

    iex> bin2 = CImg.to_flat(img, dtype: "<f4")
    # convert pixel value to 32bit-float in range 0.0..1.0.
    ```
  """
  def to_flat(cimg, opts \\ []) do
    dtype    = Keyword.get(opts, :dtype, "<f4")
    {lo, hi} = Keyword.get(opts, :range, {0.0, 1.0})
    nchw     = :nchw in opts
    bgr      = :bgr  in opts

    with {:ok, bin} <- NIF.cimg_to_bin(cimg, dtype, lo, hi, nchw, bgr) do
      %{descr: dtype, fortran_order: false, shape: {size(cimg)}, data: bin}
    end
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
