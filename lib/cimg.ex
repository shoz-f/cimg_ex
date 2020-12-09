defmodule CImg do
  @moduledoc """
  CImg image processing extention.
  """
  alias __MODULE__
  
  # image object
  defstruct handle: nil

  defmodule NIF do
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
    def cimg_create(_s),        do: raise "NIF cimg_create/1 not implemented"
    def cimg_save(_c,_s),       do: raise "NIF cimg_save/2 not implemented"
    def cimg_get_wh(_c),        do: raise "NIF cimg_get_wh/1 not implemented"
    def cimg_resize(_c,_x,_y),  do: raise "NIF cimg_resize/3 not implemented"
    def cimg_mirror(_c,_axis),  do: raise "NIF cimg_mirror/2 not implemented"
    def cimg_get_gray(_c,_pn),  do: raise "NIF cimg_get_gray/2 not implemented"
    def cimg_get_flatbin(_c),   do: raise "NIF cimg_get_flatbin/1 not implemented"
    def cimg_draw_box(_c,_x0,_y0,_x1,_y1,_rgb),
                                 do: raise "NIF cimg_draw_box/6 not implemented"
  end

  @doc """
  load the image file and create new image object.
  """
  def create(fname) do
    with {:ok, h} <- NIF.cimg_create(fname)
    do
      %CImg{
        handle: h
      }
    end
  end

  @doc "save image object to the file"
  def save(%CImg{}=cimg, fname),    do: NIF.cimg_save(cimg, fname)
  
  @doc "get width and height of the image object"
  def get_wh(%CImg{}=cimg),         do: NIF.cimg_get_wh(cimg)
  
  @doc "resize the image object"
  def resize(%CImg{}=cimg, [x, y]), do: NIF.cimg_resize(cimg, x, y)

  @doc "mirroring the image object on the axis"
  def mirror(%CImg{}=cimg, axis) when axis in [:x, :y] do
    NIF.cimg_mirror(cimg, axis)
  end

  @doc """
  create new gray image object from the image object
  """
  def get_gray(%CImg{}=cimg, opt_pn \\ 0) do
    with {:ok, gray} <- NIF.cimg_get_gray(cimg, opt_pn)
    do
      %CImg{
        handle: gray
      }
    end
  end
  
  @doc """
  get the flat binary from the image object
  """
  def to_flatbin(%CImg{}=cimg) do
    with \
      {:ok, bin} <- NIF.cimg_get_flatbin(cimg),
      shape      <- NIF.cimg_get_wh(cimg)
    do
      %{
        descr: "<u1",
        shape: shape,
        data:  bin
      }
    end
  end

  @doc """
  draw the colored box on the image object
  """
  def draw_box(%CImg{}=cimg, x0, y0, x1, y1, {_r, _g, _b}=rgb) do
    NIF.cimg_draw_box(cimg, x0, y0, x1, y1, rgb)
  end
end
