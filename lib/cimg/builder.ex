defmodule CImg.Builder do

  alias __MODULE__
  alias CImg.NIF

  # builder object
  #   :src - source image
  #   :tpircs - IP script (reversed)
  defstruct src: nil, script: []
end
