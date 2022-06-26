defmodule CImg.NIF do
  @moduledoc false
  @compile {:autoload, false}

  #loading NIF library
  @on_load :load_nif
  def load_nif do
    nif_file = Application.app_dir(:cimg, "priv/cimg_nif")
    :erlang.load_nif(nif_file, 0)
  end

  # stub implementations for NIFs (fallback)
  def cimg_run(_1),
    do: raise("NIF cimg_run/1 not implemented")
  def cimgdisplay_create(_1, _2, _3, _4, _5),
    do: raise("NIF cimgdisplay_create/5 not implemented")
  def cimgdisplay_wait(_1),
    do: raise("NIF cimgdisplay_wait/1 not implemented")
  def cimgdisplay_wait_time(_1, _2),
    do: raise("NIF cimgdisplay_wait_time/2 not implemented")
  def cimgdisplay_is_closed(_1),
    do: raise("NIF cimgdisplay_is_closed/1 not implemented")
  def cimgdisplay_button(_1),
    do: raise("NIF cimgdisplay_button/1 not implemented")
  def cimgdisplay_mouse_y(_1),
    do: raise("NIF cimgdisplay_mouse_y/1 not implemented")
end
