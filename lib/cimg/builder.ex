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
  defstruct handle: nil, script: []

  def push_cmd(%Builder{script: script}=builder, cmd) do
    %{builder| script: [cmd|script]}
  end

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
  def builder() do
    %Builder{}
  end
  def builder(%Builder{}=builder) do
    builder
  end
  def builder(%CImg{}=cimg) do
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
  def create(x, y, z, c, val) do
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
  def runit(%Builder{handle: h, script: script}) do
    stack  = [%CImg{handle: h}]
    script = Enum.reverse(script)

    runit_loop(stack, script)
  end

  def apply(%Builder{script: script}, %CImg{}=cimg) do
    stack = [CImg.dup(cimg)]
    script = Enum.reverse(script)
    
    runit_loop(stack, script)
  end

  def runit_loop([top|stack], []) do
    top
  end
  def runit_loop([top|stack], [cmd|script]) do
    res = case cmd do
      {:load, fname}               ->
        CImg.load(fname)
      {:resize, x, y, align, fill} ->
        CImg.resize(top, {x, y}, align, fill)
      {:gray, opt_pn}              ->
        with {:ok, gray} <- NIF.cimg_get_gray(top, opt_pn), do: %CImg{handle: gray}
      {:draw_circle, x0, y0, radius, color, opacity} ->
        NIF.cimg_draw_circle_filled(top, x0, y0, radius, color, opacity)
      {:draw_circle, x0, y0, radius, color, opacity, pattern} ->
        NIF.cimg_draw_circle(top, x0, y0, radius, color, opacity, pattern)
      _ -> nil
    end
    stack = [res|stack]
    runit_loop(stack, script)
  end
  
  def load(fname) do
    %Builder{script: [{:load, fname}]}
  end


  def resize(%Builder{}=builder, {x, y}=_size, align \\ :none, fill \\ 0) do
    push_cmd(builder, {:resize, x, y, align, fill})
  end

  
  def gray(%Builder{}=builder, opt_pn) do
    push_cmd(builder, {:gray, opt_pn})
  end

  def draw_circle(%Builder{}=builder, x0, y0, radius, color, opacity \\ 1.0) do
    push_cmd(builder, {:draw_circle, x0, y0, radius, color, opacity})
  end
  
  def draw_circle(%Builder{}=builder, x0, y0, radius, color, opacity, pattern) do
    push_cmd(builder, {:draw_circle, x0, y0, radius, color, opacity, pattern})
  end
end
