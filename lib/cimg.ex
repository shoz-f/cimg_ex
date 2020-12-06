defmodule CImg do
  @moduledoc """
  Documentation for `CImg`.
  """
  
  defstruct handle: nil
  
  def create(fname) do
    with {:ok, h} <- cimg_create(fname)
    do
      %CImg{
        handle: h
      }
    end
  end

  def save(%CImg{handle: h}, fname) do
    cimg_save(h, fname)
    :ok
  end

  def get_wh(%CImg{handle: h}) do
    cimg_get_wh(h)
  end

  def resize(%CImg{handle: h}=img, [x, y]) do
    cimg_resize(h, x, y)
    img
  end

  def mirror(%CImg{handle: h}=img, axis) when axis in [:x, :y] do
    cimg_mirror(h, axis)
    img
  end

  def get_gray(%CImg{handle: h}, opt_pn \\ 0) do
    with {:ok, gray} <- cimg_get_gray(h, opt_pn)
    do
      %CImg{
        handle: gray
      }
    end
  end
  
  def draw_box(%CImg{handle: h}=img, x0, y0, x1, y1, {_r, _g, _b}=rgb) do
    cimg_draw_box(h, x0, y0, x1, y1, rgb)
    img
  end

  # loading NIF library
  @on_load :load_nif
  def load_nif do
    nif_file = Application.app_dir(:cimg, "priv/cimg_nif")
    :erlang.load_nif(nif_file, 0)
  end

  # stub implementations for NIFs (fallback)
  def cimg_hello(),           do: raise "NIF cimg_hello/0 not implemented"
  def cimg_create(_s),        do: raise "NIF cimg_create/1 not implemented"
  def cimg_save(_h,_s),       do: raise "NIF cimg_save/2 not implemented"
  def cimg_get_wh(_h),        do: raise "NIF cimg_get_wh/1 not implemented"
  def cimg_resize(_h,_x,_y),  do: raise "NIF cimg_resize/3 not implemented"
  def cimg_mirror(_h,_axis),  do: raise "NIF cimg_mirror/2 not implemented"
  def cimg_get_gray(_h,_pn),  do: raise "NIF cimg_get_gray/2 not implemented"
  def cimg_draw_box(_h,_x0,_y0,_x1,_y1,_rgb),
                               do: raise "NIF cimg_draw_box/6 not implemented"
end
