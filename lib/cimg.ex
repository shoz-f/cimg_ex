defmodule CImg do
  @moduledoc """
  A lightweight image processing module in Elixir using CImg, aimed at creating auxiliary routines for Deep Learning.

  ### Session execution

  Each function provided by CImg can be used alone (eager execution) or
  as a set of functions (session execution). 
  
  EAGER execution is good for interactive work because you can see the results
  of your image processing immediately. But it tends to consume more memory
  than necessary. On the other hand, SESSION execution reduces memory consumption
  because after assembling the image processing sequence, it is executed all at once.
  
  To perform image processing in SESSION mode, you must assemble a image processing
  sequence using Elixir's pipe syntax. At the entrance of the pipe, place
  a function {seed} that generates a %Builder{}, then pipe in the necessary
  image processing {grow}, and finally place a function {crop} that executes
  those image processing.
  
  In this document, the following symbols indicate which category each function belongs to.
    * {seed} - generate %Builder{}
    * {grow} - image processing piece
    * {crop} - execute session and get result

  #### Examples
  
     ```elixir
    img = CImg.load("sample.jpg")

    CImg.builder(img)               # generate %Builder{} with %CImg{} as seed image
    |> CImg.draw_circle(100, 100, 30, {0,0,255}) # building an image processing session
    |> CImg.draw_circle(150, 200, 40, {255,0,0}) # ...
    |> CImg.display(disp)           # execute above session and display result image on PC screen
    ```

  ### Data exchange with Nx
  It is easy to exchange %CImg{} and Nx tensors.

  ```elixir
  # convert CImg image to Nx.Tensor
  iex> img0 = CImg.load("sample.jpg")
  %CImg{{2448, 3264, 1, 3}, handle: #Reference<0.2112145695.4205182979.233827>}
  iex> tensor = CImg.to_binary(img0, dtype: "<u8")
         |> Nx.from_binary({:u, 8})

  # convert Nx.Tensor to CImg image
  iex> img1 = Nx.to_binary(tensor)
         |>CImg.from_bin(2448, 3264, 1, 3, "<u8")
  ```
  """
  alias __MODULE__
  alias CImg.NIF

  # image object
  #   :handle - Erlang resource object pointing to the CImg image.
  defstruct handle: nil

  defimpl Inspect do
    import Inspect.Algebra

    def inspect(cimg, opts) do
      concat(["%CImg{", to_doc(CImg.shape(cimg), opts), ", handle: ", to_doc(cimg.handle, opts), "}"])
    end
  end

  defmodule Builder do
    @moduledoc """
    Record the image processing sequence for SESSION execution.
    """
	  # builder object
	  #   :seed   - operation to get a initial image.
	  #   :script - image operations
	  defstruct seed: nil, script: []
  end

  defp push_cmd(%Builder{script: script}=builder, cmd) do
    %{builder| script: [cmd|script]}
  end

  @doc """
  {seed} Returns an empty builder for recording image processing scripts.

  ## Parameters

    None.

  ## Examples

    ```elixir
    # build a script
    script = CImg.builder()
      |> CImg.gray()
      |> CImg.resize({256, 256})

    # apply the script to an image.
    seed   = CImg.load("sample.jpg")
    result = CImg.run(script, seed)
    ```
  """
  def builder() do
    %Builder{}
  end

  @doc """
  {seed} Returns a builder with an existing %CImg{} as the seed image.

  ## Parameters

    * img - %CImg{}

  ## Examples

    ```elixir
    cimg = CImg.load("sample.jpg")

    result = CImg.builder(cimg)  # create %Builder{} has cimg
      |> CImg.draw_circle(100, 100, 30, {0, 255, 0})
      |> CImg.run()
    ```
  """
  def builder(%CImg{}=cimg) do
    %Builder{seed: {:copy, cimg}}
  end


  @doc """
  {seed} Returns a builder that uses an image read from the file as the seed image.

  ## Parameters

    * atom - image file format: `:jpeg` or `:png`
    * fname - file name

  ## Examples

    ```elixir
    result = CImg.builder(:jpeg, "sample.jpg")
      |> CImg.draw_circle(100, 100, 30, {0, 255, 0})
      |> CImg.run()
    ```
  """
  def builder(:file, fname) do
    %Builder{seed: {:load, fname}}
  end

  def builder(:image, jpeg_or_png) do
    %Builder{seed: {:load_from_memory, jpeg_or_png}}
  end


  @doc """
  {seed} Returns a builder whose seed image is an image of shape [x,y,z,c] filled with the value `val`.

  ## Parameters

    * x,y,z,c - shape of image
    * val - filling value

  ## Examples

    ```elixir
    res = CImg.builder(640, 480, 1, 3, 64)
      |> CImg.draw_circle(100, 100, 30, {0, 255, 0})
      |> CImg.run()
    ```
  """
  def builder(x, y, z, c, val) when is_integer(val) do
    %Builder{seed: {:create, x, y, z, c, val}}
  end


  @doc """
  {seed} Returns a builder that takes as seed image a binary - shape[], dtype -
  converted to an image

  ## Parameters

    * bin - binary
    * x,y,z,c - shape of the image represented by `bin`
    * dtype - data type of bin: `"<f4"`/32bit-float, `"<u1"`/8bit-unsigned-int

  ## Examples

    ```elixir
    result = CImg.builder(bin 640, 480, 1, 3, "<f4")
      |> CImg.draw_circle(100, 100, 30, {0, 255, 0})
      |> CImg.run()
    ```
  """
  def builder(bin, x, y, z, c, dtype) when is_binary(bin) do
    %Builder{seed: {:create_from_bin, bin, x, y, z, c, dtype}}
  end


  @doc """
  {crop} Returns an image with the script applied to the seed image.
  
  ## Parameters

    * builder - %Builder{}

  ## Examples

    ```elixir
    result = CImg.builder(100, 100, 1, 3, 0)
      |> CImg.draw_circle(100, 100, 30, {0, 255, 0})
      |> CImg.run()
    ```
  """
  def run(%Builder{seed: seed, script: script}) when not is_nil(seed) do
    script = [{:get_image} | script]
    with {:ok, img} <- NIF.cimg_run([seed | Enum.reverse(script)]),
      do: %CImg{handle: img}
  end


  @doc """
  {crop} Returns an image with the script applied to `cimg`.

  ## Parameters

    * builder - %Builder{}
    * cimg - %CImg{}

  ## Examples

    ```elixir
    cimg = CImg.load("sample.jpg")

    result = CImg.builder()
      |> CImg.draw_circle(100, 100, 30, {0, 255, 0})
      |> CImg.runit(cimg)
    ```
  """
  def run(%Builder{}=builder, %CImg{}=cimg) do
    run(%Builder{builder | seed: {:copy, cimg}})
  end


  @doc """
  Create image{x,y,z,c} filled `val`.

  ## Parameters

    * x,y,z,c - image's x-size, y-size, z-size and spectrum.
    * val     - filling value.

  ## Examples

    ```elixir
    img = CImg.create(200, 100, 1, 3, 127)
    ```
  """
  def create(x, y, z, c, val) when is_integer(val) do
    builder(x, y, z, c, val) |> run()
  end

  
  @doc """
  Load a image from file. The file types supported by this function are jpeg, ping and bmp.
  The file extension identifies which file type it is.

  ## Parameters

    * fname - file path of the image.

  ## Examples

    ```elixir
    img = CImg.load("sample.jpg")
    ```
  """
  def load(fname) do
    builder(:file, fname) |> run()
  end


  @doc """
  Create a image from jpeg/png format binary.
  You can create an image from loaded binary of the JPEG/PNG file.

  ## Parameters

    * jpeg_or_png - loaded binary of the image file.

  ## Examples

    ```elixir
    bin  = File.read!("sample.jpg")
    jpeg = CImg.from_binary(bin)
    ```
  """
  def from_binary(jpeg_or_png) do
    builder(:image, jpeg_or_png) |> run()
  end


  @doc """
  Create image{x,y,z,c} from raw binary.
  `create_from_bin` helps you to make the image from the serialiezed output tensor of DNN model.

  ## Parameters

    * bin - raw binary data to have in a image.
    * x,y,z,c - image's x-size, y-size, z-size and spectrum.
    * dtype - data type in the binary. any data types are converted to int8 in the image.
      - "<f4" - 32bit float (available value in range 0.0..1.0)
      - "<u1" - 8bit unsigned integer

  ## Examples

    ```elixir
    bin = TflInterp.get_output_tensor(__MODULE__, 0)
    img = CImg.create_from_bin(bin, 300, 300, 1, 3, "<f4")
    ```
  """
  def from_binary(bin, x, y, z, c, dtype) when is_binary(bin) do
    builder(bin, x, y, z, c, dtype) |> run()
  end


  @doc """
  Create the image from %Npy{} format data.

  ## Parameters

    * npy - %Npy{} has 3 rank.

  ## Examples

    ```elixir
    {:ok, npy} = Npy.load("image.npy")
    img = CImg.from_npy(npy)
    ```
  """
  def from_npy(%{descr: dtype, shape: {h, w, c}, data: bin}) do
    from_binary(bin, w, h, 1, c, dtype)
  end


  @doc """
  {crop} Get shape {x,y,z,c} of the image

  ## Parameters

    * img - %CImg{} or %Builder{}

  ## Examples

    ```elixir
    shape = CImg.shape(img)
    ```
  """
  def shape(%CImg{}=img) do
    builder(img) |> shape()
  end

  def shape(%Builder{seed: seed, script: script}) when not is_nil(seed) do
    script = [{:get_shape} | script]
    NIF.cimg_run([seed | Enum.reverse(script)])
  end


  @doc """
  {crop} Get byte size of the image

  ## Parameters

    * img - %CImg{} or %Builder{}

  ## Examples

    ```elixir
    size = CImg.size(img)
    ```
  """
  def size(%CImg{}=img) do
    builder(img) |> size()
  end

  def size(%Builder{seed: seed, script: script}) when not is_nil(seed) do
    script = [{:get_size} | script]
    NIF.cimg_run([seed | Enum.reverse(script)])
  end


  @doc """
  {crop} Save the image to the file.

  ## Parameters

    * img - %CImg{} or %Builder{}
    * fname - file path for the image. (only jpeg images - xxx.jpg - are available now)

  ## Examples

    ```elixir
    CImg.save(img, "sample.jpg")
    ```
  """
  def save(%CImg{}=img, fname) do
    builder(img)
    |> save(fname)
  end

  def save(%Builder{seed: seed, script: script}, fname)  when not is_nil(seed) do
    script = [{:save, fname} | script]
    NIF.cimg_run([seed | Enum.reverse(script)])
  end


  @doc """
  {crop} Get serialized binary of the image from top-left to bottom-right.
  `to_binary/2` helps you to make 32bit-float arrary for the input tensors of DNN model
  or jpeg/png format binary on memory.

  ## Parameters

    * img - %CImg{} or %Builder{}
    * opts - conversion options
      - :jpeg - convert to JPEG format binary.
      - :png - convert to PNG format binary.

      following options can be applied when converting the image to row binary.
      - { :dtype, xx } - convert pixel value to data type.
           available: "<f4"/32bit-float, "<u1"/8bit-unsigned-char
      - { :range, {lo, hi} } - normarilzed range when :dtype is "<f4".
           default range: {0.0, 1.0}
      - :nchw - transform axes NHWC to NCHW.
      - :bgr - convert color RGB -> BGR.

  ## Examples

    ```elixir
    img = CImg.load("sample.jpg")

    jpeg = CImg.to_binary(img, :jpeg)
    # convert to JPEG format binary on memory.

    png = CImg.to_binary(img, :png)
    # convert to PNG format binary on memory.

    bin1 = CImg.to_binary(img, [{dtype: "<f4"}, {:range, {-1.0, 1.0}}, :nchw])
    # convert pixel value to 32bit-float in range -1.0..1.0 and transform axis to NCHW.

    bin2 = CImg.to_binary(img, dtype: "<f4")
    # convert pixel value to 32bit-float in range 0.0..1.0.
    ```
  """
  def to_binary(img, opts \\ [])

  def to_binary(%CImg{}=cimg, opts) do
    builder(cimg)
    |> to_binary(opts)
  end

  def to_binary(%Builder{seed: seed, script: script}, opts) when opts in [:jpeg, :png] do
    script = [{:to_image, opts} | script]
    with {:ok, image} <- NIF.cimg_run([seed | Enum.reverse(script)]),
      do: image
  end

  def to_binary(%Builder{seed: seed, script: script}, opts) do
    dtype    = Keyword.get(opts, :dtype, "<f4")
    {lo, hi} = Keyword.get(opts, :range, {0.0, 1.0})
    nchw     = :nchw in opts
    bgr      = :bgr  in opts

    script = [{:to_bin, dtype, lo, hi, nchw, bgr} | script]
    with {:ok, _shape, bin} <- NIF.cimg_run([seed | Enum.reverse(script)]),
      do: bin
  end


  @doc """
  {crop} Convert the image to %Npy{} format data.

  ## Parameters

    * img - %CImg{} or %Builder{}
    * opts - conversion options
      - { :dtype, xx } - convert pixel value to data type.
           available: "<f4"/32bit-float, "<u1"/8bit-unsigned-char
      - { :range, {lo, hi} } - normarilzed range when :dtype is "<f4".
           default range: {0.0, 1.0}
      - :nchw - transform axes NHWC to NCHW.
      - :bgr - convert color RGB -> BGR.

  ## Examples

    ```elixir
    img = CImg.load("sample.jpg")

    npy1 =
      img
      |> CImg.to_npy()

    npy2 =
      img
      |> CImg.to_npy([{dtype: "<f4"}, {:range, {-1.0, 1.0}}, :nchw])
    # convert pixel value to 32bit-float in range -1.0..1.0 and transform axis to NCHW.
    ```
  """
  def to_npy(img, opts \\ [])

  def to_npy(%CImg{}=cimg, opts) do
    builder(cimg)
    |> to_npy(opts)
  end

  def to_npy(%Builder{seed: seed, script: script}, opts) do
    dtype    = Keyword.get(opts, :dtype, "<f4")
    {lo, hi} = Keyword.get(opts, :range, {0.0, 1.0})
    nchw     = :nchw in opts
    bgr      = :bgr  in opts

    script = [{:to_bin, dtype, lo, hi, nchw, bgr} | script]
    with {:ok, shape, bin} <- NIF.cimg_run([seed | Enum.reverse(script)]) do
      %{
        descr: dtype,
        fortran_order: false,
        shape: shape,
        data: bin
      }
    end
  end


  @doc """
  {crop} Get the pixel value at (x, y).

  ## Parameters

    * img - %CImg{} or %Builder{}
    * x,y,z,c - location in the image.

  ## Examples

    ```elixir
    val = CImg.get(img, 120, 40)
    ```
  """
  def get(img, x, y \\ 0, z \\ 0, c \\ 0)

  def get(%CImg{}=cimg, x, y, z, c) do
    builder(cimg) |> get(x, y, z, c)
  end

  def get(%Builder{seed: seed, script: script}, x, y, z, c) do
    script = [{:get, x, y, z, c} | script]
    NIF.cimg_run([seed | Enum.reverse(script)])
  end


  @doc """
  {crop} Extracting a partial image specified in a window from an image.

  ## Parameters

    * img - %CImg{} or %Builder{}
    * x0,y0,z0,c0, x1,y1,z1,c1 - window
    * bundary_condition -
  
  ## Examples
  
    ```elixir
    partial = CImg.get_crop(img, 100, 100, 0, 0, 400, 600, 0, 3)
    ```
  """
  def get_crop(img, x0, y0, z0, c0, x1, y1, z1, c1, boundary_conditions \\ 0)

  def get_crop(%CImg{}=cimg, x0, y0, z0, c0, x1, y1, z1, c1, boundary_conditions) do
    builder(cimg)
    |> get_crop(x0, y0, z0, c0, x1, y1, z1, c1, boundary_conditions)
  end

  def get_crop(%Builder{seed: seed, script: script}, x0, y0, z0, c0, x1, y1, z1, c1, boundary_conditions) do
    script = [{:get_crop, x0, y0, z0, c0, x1, y1, z1, c1, boundary_conditions} | script]
    with {:ok, img} <- NIF.cimg_run([seed | Enum.reverse(script)]),
      do: %CImg{handle: img}
  end


  @doc """
  {grow} Booking to clear the image.

  ## Parameters

    * builder - %Builder{}.

  ## Examples

  ```elixir
  result = CImg.builder(:file, "sample.jpg")
    |> CImg.clear()
    |> CImg.run()
  ```
  """
  def clear(%Builder{}=builder) do
    push_cmd(builder, {:clear})
  end


  @doc """
  {grow} Booking to fill the image with `val`.

  ## Parameters

    * builder - %Builder{}
    * val - filling value.

  ## Examples

    ```Elixir
    result = CImg.builder(cimg)
      |> CImg.fill(img, 0x7f)
      |> CImg.run()
    ```
  """
  def fill(%Builder{}=builder, val) do
    push_cmd(builder, {:fill, val})
  end


  @doc """
  {grow} Get the inverted image of the image.

  ## Examples

    ```elixir
    inv = CImg.invert(img)
    # get inverted image
    ```
  """
  def invert(%CImg{}=img) do
    builder(img)
    |> invert()
    |> run()
  end

  def invert(%Builder{}=builder) do
    push_cmd(builder, {:invert})
  end


  @doc """
  {grow} Get the gray image of the image.

  ## Parameters

    * img - %CImg{} or %Builder{}
    * opt_pn - intensity inversion: 0 (default) - no-inversion, 1 - inversion

  ## Examples

    ```elixir
    gray = CImg.gray(img)
    ```
  """
  def gray(img, opt_pn \\ 0)

  def gray(%CImg{}=cimg, opt_pn) do
    builder(cimg)
    |> gray(opt_pn)
    |> run()
  end

  def gray(%Builder{}=builder, opt_pn) do
    push_cmd(builder, {:gray, opt_pn})
  end


  @doc """
  {grow} Thresholding the image.

  ## Parameters

    * img - %CImg{} or %Builder{} object.
    * val - threshold value
    * soft -
    * strict -

  ## Examples

    ```Elixir
    res = CImg.threshold(img, 100)
    ```
  """
  def threshold(img, val, soft \\ false, strict \\ false)

  def threshold(%CImg{}=cimg, val, soft, strict) do
    builder(cimg)
    |> threshold(val, soft, strict)
    |> run()
  end

  def threshold(%Builder{}=builder, val, soft, strict) do
    push_cmd(builder, {:threshold, val, soft, strict})
  end


  @doc """
  {grow} Create color mapped image by lut.

  ## Parameters

    * img - %CImg{} or %Builder{}
    * lut - color map. build-in or user defined.
      - build-in map: {:default, :lines, :hot, :cool, :jet}
      - user defined: list of color tupple, [{0,0,0},{10,8,9},{22,15,24}...].
    * boundary - handling the pixel value outside the color map range.
      - 0 - set to zero value.
      - 1 -
      - 2 - repeat from the beginning of the color map.
      - 3 - repeat while wrapping the color map.

  ## Examples

    ```elixir
    gray = CImg.load("sample.jpg") |> CImg.gray()

    jet = CImg.color_mapping(gray, :jet)
    # heat-map coloring.

    custom = CImg.color_mapping(gray, [{0,0,0},{10,8,9},{22,15,24}], 2)
    # custom coloring.
    ```
  """
  def color_mapping(img, lut \\ :default, boundary \\ 0)

  def color_mapping(%CImg{}=cimg, lut, boundary) do
    builder(cimg)
    |> color_mapping(lut, boundary)
    |> run()
  end

  def color_mapping(%Builder{}=builder, lut, boundary) do
    cond do
      lut in [:default, :lines, :hot, :cool, :jet] ->
      	  push_cmd(builder, {:color_mapping, lut, boundary})
      is_list(lut) ->
      	  push_cmd(builder, {:color_mapping_by, lut, boundary})
    end
  end


  @doc """
  {grow} Bluring image.

  ## Parameters

    * img - %CImg{} or %Builder{}
    * sigma -
    * boundary_conditions -
    * is_gaussian -

  ## Examples

    ```elixir
    img = CImg.load("sample.jpg")
    blured = CImg.blur(img, 0.3)
    ```
  """
  def blur(img, sigma, boundary_conditions \\ true, is_gaussian \\ true)

  def blur(%CImg{}=cimg, sigma, boundary_conditions, is_gaussian) do
    builder(cimg)
    |> blur(sigma, boundary_conditions, is_gaussian)
    |> run()
  end

  def blur(%Builder{}=builder, sigma, boundary_conditions, is_gaussian) do
    push_cmd(builder, {:blur, sigma, boundary_conditions, is_gaussian})
  end


  @doc """
  {grow} mirroring the image on `axis`

  ## Parameters

    * img - %CImg{} or %Builder{}
    * axis - flipping axis: :x, :y

  ## Examples

    ```elixir
    mirror = CImg.mirror(img, :y)
    # vertical flipping
    ```
  """
  def mirror(%CImg{}=img, axis) do
    builder(img)
    |> mirror(axis)
    |> run()
  end

  def mirror(%Builder{}=builder, axis) when axis in [:x, :y] do
  	push_cmd(builder, {:mirror, axis})
  end


  @doc """
  {grow} transpose the image

  ## Parameters

    * img - %CImg{} or %Builder{}

  ## Examples

    ```elixir
    transposed = CImg.transpose(img)
    ```
  """
  def transpose(%CImg{}=img) do
    builder(img)
    |> transpose()
    |> run()
  end

  def transpose(%Builder{}=builder) do
    push_cmd(builder, {:transpose})
  end


  @doc """
  {grow} Get a new image object resized {x, y}.

  ## Parameters

    * cimg - image object.
    * {x, y} - resize width and height or
      scale  - resize scale
    * align - alignment mode
      - :none - fit resizing
      - :ul - fixed aspect resizing, upper-leftt alignment.
      - :br - fixed aspect resizing, bottom-right alignment.
    * fill - filling value for the margins, when fixed aspect resizing.

  ## Examples

    ```elixir
    img = CImg.load("sample.jpg")
    result = CImg.get_resize(img, {300,300}, :ul)
    ```
  """
  def resize(img, size, align \\ :none, fill \\ 0)

  def resize(%CImg{}=cimg, scale, align, fill) when is_float(scale) do
    size_xy = -round(100*scale)
    resize(cimg, {size_xy, size_xy}, align, fill)
  end

  def resize(%CImg{}=cimg, size, align, fill) when tuple_size(size) == 2 do
    builder(cimg)
    |> resize(size, align, fill)
    |> run()
  end

  def resize(%Builder{}=builder, {x, y}, align, fill) do
    align = case align do
      :none -> 0
      :ul   -> 1
      :br   -> 2
      _     -> raise(ArgumentError, "unknown align '#{align}'.")
    end

    push_cmd(builder, {:resize, x, y, align, fill})
  end


  @doc """
  {grow} Set the pixel value at (x, y).

  ## Parameters

    * builder - %Builder{}
    * val - value.
    * x,y,z,c - location in the image.

  ## Examples

    ```elixir
    result = CImg.builder(cimg)
      |> CImg.set(0x7f, 120, 40)
      |> CImg.run()
    ```
  """
  def set(%Builder{}=builder, x, y \\ 0, z \\ 0, c \\ 0, val) do
    push_cmd(builder, {:set, x, y, z, c, val})
  end


  @doc """
  {grow} Booking to draw rectangle in the image.

  ## Parameters

    * builder - %Builder{}
    * x0,y0,x1,y1 - diagonal coordinates. if all of them are integer, they mean
    actual coodinates. if all of them are float within 0.0-1.0, they mean ratio
    of the image.
    * color - boundary color
    * opacity - opacity: 0.0-1.0
    * pattern - boundary line pattern: 32bit pattern

  ## Examples

    ```elixir
    CImg.builder(cimg)
    |> CImg.draw_rect(img, 50, 30, 100, 80, {255, 0, 0}, 0.3, 0xFF00FF00)
    |> CImg.display()

    CImg.builder(cimg)
    |> CImg.draw_rect(img, 0.2, 0.3, 0.6, 0.8, {0, 255, 0})
    |> CImg.display()
    ```
  """
  def draw_rect(%Builder{}=builder, x0, y0, x1, y1, color, opacity \\ 1.0, pattern \\ 0xFFFFFFFF) do
    cond do
      Enum.all?([x0, y0, x1, y1], &is_integer/1) ->
        push_cmd(builder, {:draw_rectangle, x0, y0, x1, y1, color, opacity, pattern})
      Enum.all?([x0, y0, x1, y1], fn x -> 0.0 <= x and x <= 1.0 end) ->
      	push_cmd(builder, {:draw_rectangle_ratio, x0, y0, x1, y1, color, opacity, pattern})
    end
  end


  @doc """
  {grow} Booking to draw filled circle in the image.

  ## Parameters

    * builder - %Builder{}
    * x0,y0 - circle center location
    * radius - circle radius
    * color - filling color
    * opacity - opacity: 0.0-1.0

  ## Examples

    ```elixir
    result = CImg.builder(cimg)
      |> CImg.draw_circle(imge, 100, 80, 40, {0, 0, 255})
      |> CImg.run()
    ```
  """
  def draw_circle(%Builder{}=builder, x0, y0, radius, color, opacity \\ 1.0) do
    push_cmd(builder, {:draw_circle_filled, x0, y0, radius, color, opacity})
  end


  @doc """
  {grow} Booking to draw circle in the image.

  ## Parameters

    * builder - %Builder{}
    * x0,y0 - circle center location
    * radius - circle radius
    * color - boundary color
    * opacity - opacity: 0.0-1.0
    * pattern - boundary line pattern

  ## Examples

    ```Elixir
    result = CImg.builder(cimg)
      |> CImg.draw_circle(imge, 100, 80, 40, {0, 0, 255}, 0.3, 0xFFFFFFFF)
      |> CImg.run()
    ```
  """
  def draw_circle(%Builder{}=builder, x0, y0, radius, color, opacity, pattern) do
    push_cmd(builder, {:draw_circle, x0, y0, radius, color, opacity, pattern})
  end


  @doc """
  {grow} Booking to draw graph.

  ## Parameters

    * builder - %Builder{}
    * data - plot data (%CImg{})
    * color - RGB color tuple: {R,G,B} where 0 ≦ R,G,B ≦ 255
    * opacity -
    * plot_type - 
      * 0 = No plot.
      * 1 = Plot using segments.
      * 2 = Plot using cubic splines.
      * 3 = Plot with bars.
    * vertex_type - 
      * 0 = No points.
      * 1 = Point.
      * 2 = Straight cross.
      * 3 = Diagonal cross.
      * 4 = Filled circle.
      * 5 = Outlined circle.
      * 6 = Square.
      * 7 = Diamond.
    * ymin, ymax - lower and upper bound of the y-range
    * pattern - line style

  ## Examples
  
    ```elixir
      CImg.builder(screen)
        |> CImg.draw_graph(CImg.get_crop(image, 0, y, 0, 0, width-1, y, 0, 0), red,   1.0, 1, 0, 255.0, 0.0)
        |> CImg.draw_graph(CImg.get_crop(image, 0, y, 0, 1, width-1, y, 0, 1), green, 1.0, 1, 0, 255.0, 0.0)
        |> CImg.draw_graph(CImg.get_crop(image, 0, y, 0, 2, width-1, y, 0, 2), blue,  1.0, 1, 0, 255.0, 0.0)
        |> CImg.display(draw_disp)
    ```
  """
  def draw_graph(%Builder{}=builder, data, color, opacity \\ 1.0, plot_type \\ 1, vertex_type \\ 1, ymin \\ 0.0, ymax \\ 0.0, pattern \\ 0xFFFFFFFF) do
    push_cmd(builder, {:draw_graph, data, color, opacity, plot_type, vertex_type, ymin, ymax, pattern})
  end


  @doc """
  {grow} Booking to move pixels on the image according to the mapping table.

  ## Parameters

    * img - %CImg{} or %Builder{}
    * mapping - mapping table. ex) [{[10,10],[10,20]}], move pixel at [10,10] to [10,20]
    * cx, cy, cz - location of upper-left mapping table on both images.

  ## Examples

    ```elixir
    map = [{[20,20],[25,25]}, {[20,21],[25,26]}]

    result = CImg.builder(:jpeg, "sample.jpg")
      |> CImg.draw_morph(map)
      |> CImg.run()
    ```
  """
  def draw_morph(img, mapping, cx \\ 0, cy \\ 0, cz \\ 0)

  def draw_morph(%CImg{}=cimg, mapping, cx, cy, cz) do
    builder(cimg)
    |> draw_morph(mapping, cx, cy, cz)
    |> run()
  end

  def draw_morph(%Builder{}=builder, mapping, cx, cy, cz) do
    push_cmd(builder, {:draw_morph, mapping, cx, cy, cz})
  end


  @doc """
  {grow} Draw text in th image.
  
  ## Parameters
  
    * builder - %Builder{}
    * x,y - position on the image where the text will begin to be drawn.
    * text - the text to be drawn.
    * font_height - font height in pixels.
    * fg_color - foreground color. choose one of the following:
            :white,:sliver,:gray,:black,:red,:maroon,:yellow,:olive,
            :lime,:green,:aqua,:teal,:blue,:navy,:fuchsia,:purple,
            :transparent
    * bg_color - background color.
    * opacity - opacity: 0.0..1.0.

  ## Examples
  
    ```elixir
    result = CImg.draw_text(builder, 10, 20, "Hello world!", 32, :white)
    ```
  """
  def draw_text(%Builder{}=builder, x, y, text, font_height, fg_color, bg_color \\ :transparent, opacity \\ 1.0) do
    push_cmd(builder, {:draw_text, x, y, text, fg_color, bg_color, opacity, font_height})
  end


  @doc """
  {crop} Display the image on the CImgDisplay object.

  ## Parameters

    * img - %CImg{} or %Builder{}
    * display - CImgDisplay object

  ## Examples

    ```elixir
    CImg.display(cimg)

    CImg.builder(cimg)
    |> CImg.draw_circle(100, 100, 50, {255, 0, 0})
    |> CImg.display()
    ```
  """
  def display(%CImg{}=img) do
    builder(img) |> display()
  end

  def display(%Builder{seed: seed, script: script}) do
    script = [{:display} | script]
    NIF.cimg_run([seed | Enum.reverse(script)])
  end


  @doc """
  {crop} Display the image on the CImgDisplay object.

  ## Parameters

    * cimg - %CImg{} or %Builder{}
    * display - CImgDisplay object

  ## Examples

    ```elixir
    disp = CImgDisplay.create(img, "Sample")
    CImg.display(img, disp)
    ```
  """
  def display(%CImg{}=img, disp) do
    builder(img) |> display(disp)
  end

  def display(%Builder{seed: seed, script: script}, disp) when not is_nil(seed) do
    script = [{:display_on, disp} | script]
    NIF.cimg_run([seed | Enum.reverse(script)])
  end

  @doc """
  {crop} Display the image on the Livebook.

  ## Parameters

    * img - %CImg{} or %Builder{}
    * mime_type - image file format: `:jpeg`, `:png`

  ## Examples

    ```elixir
    CImg.builder(:file, "sample.jpg")
    |> CImg.display_kino(:jpeg)
    ```
  """
  def display_kino(%CImg{}=img, mime_type) do
    builder(img) |> display_kino(mime_type)
  end

  def display_kino(%Builder{}=builder, mime_type) when mime_type in [:jpeg, :png] do
    %{
      __struct__: Kino.Image,
      content: to_binary(builder, mime_type),
      mime_type: "image/#{Atom.to_string(mime_type)}"
    }
  end
end
