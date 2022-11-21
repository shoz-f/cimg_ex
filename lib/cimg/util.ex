defmodule CImg.Util do
  @moduledoc """
  CImg utility library.
  """
  
  @doc """
  Get aspect of the image.
  
  ## Parameters
  
    * img - %CImg{}
  """
  def aspect(%CImg{}=img) do
    {x, y, _, _} = CImg.shape(img)
    if x >= y, do: [1.0, y/x], else: [x/y, 1.0]
  end

  @doc """
  [util] Make random color palette %{}.
  
  ## Parameters
  
    * keys - keys of the color palette
      * if keys is integer -> 0..(keys-1)
      * if keys is binary  -> read from the file "keys"
      * if keys is list    -> use it
  """
  def rand_palette(n) when is_integer(n), do:
    Enum.to_list(0..(n-1))
    |> rand_palette()

  def rand_palette(path) when is_binary(path) do
    for item <- File.stream!(path) do String.trim_trailing(item) end
    |> rand_palette()
  end

  def rand_palette(keys) when is_list(keys) do
    for key <- keys do {key, rand_color()} end
    |> Enum.into(%{})
  end
  
  @doc """
  [util] Get a rondom color.
  """
  def rand_color() do
    {:rand.uniform(256)-1, :rand.uniform(256)-1, :rand.uniform(256)-1}
  end
end
