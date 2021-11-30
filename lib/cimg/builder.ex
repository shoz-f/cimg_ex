defmodule CImg.Builder do
  @moduledoc """
  Build and excecute image processing sequence.
  ** under construction **
  """
  
  alias __MODULE__
  alias CImg.NIF

  # builder object
  #   :handle - work image.
  #   :src    - source image.
  #   :script - image operations
  defstruct handle: nil, src: nil, script: []

  @doc """
  Building image processing sequence. It allows to execute mutable operation in
  this sequence.
  
  ## Parameters
  
    * image - %CImg{} or %Builder{} object. if %CImg{} is passed, `builder' duplicates
    the image object and returns it wrapped with %Builder{}.
  
  ## Examples
  
    ```Elixir
    img = CImg.load("sample.jpg")
    
    res = CImg.builder(img)
      |> CImg.draw_circle(100, 100, 30, {0, 255, 0})  # draw a circle on the duplicated img.
    ```
  """
  def builder(%Builder{}=builder) do
    builder
  end
  def builder(cimg) do
    dup = CImg.dup(cimg)
    %Builder{handle: dup.handle}
  end


  @doc """
  Building image processing sequence. This one creates empty image object instead
  recieving a image.
  
  ## Parameters
  
    * x, y, z, c - image shape.
    * val - filling value
  
  ## Examples
  
    ```Elixir
    res = CImg.builder(100, 100, 1, 3, 0)   # 0 filled color image {100, 100, 1, 3}
      |> CImg.draw_circle(100, 100, 30, {0, 255, 0})  # draw a circle on the duplicated img.
    ```
  """
  def builder(x, y, z, c, val) do
    with {:ok, h} <- NIF.cimg_create(x, y, z, c, val),
      do: %Builder{handle: h}
  end


  @doc """
  Return %CImg{} converted from %Builder{}. Of course, mutable operations cannot
  be applied to %CImg{}.
  
  ## Parameters
  
    * builder - %Builder{} object.
  
  ## Examples
  
    ```Elixir
    cimg = CImg.builder(100, 100, 1, 3, 0)
      |> CImg.draw_circle(100, 100, 30, {0, 255, 0})  # draw a circle on the duplicated img.
      |> CImg.runit()

    # cimg is %CImg{} object with a circle drawn on it.
    ```
  """
  def runit(%Builder{handle: h}) do
    %CImg{handle: h}
  end
end
