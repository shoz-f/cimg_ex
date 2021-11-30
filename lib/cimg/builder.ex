defmodule CImg.Builder do

  alias __MODULE__
  alias CImg.NIF

  # builder object
  #   :handle - work image.
  #   :src    - source image.
  #   :script - image operations
  defstruct handle: nil, src: nil, script: []

  def builder(%Builder{}=builder) do
    builder
  end
  def builder(cimg) do
    dup = CImg.dup(cimg)
    %Builder{handle: dup.handle}
  end

  def builder(x, y, z, c, val) do
    with {:ok, h} <- NIF.cimg_create(x, y, z, c, val),
      do: %Builder{handle: h}
  end
  
  def runit(%Builder{handle: h}) do
    %CImg{handle: h}
  end
end
