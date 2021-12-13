defmodule CImgDisplay do
  @moduledoc """
  Display the CImg objects on PC screen. This module also provides the functionality
  to handle mouse-event, key-board-event, window-close-event, timer-event etc.
  
  Note: You can only use these features in :dev or :test mode.
  """

  alias __MODULE__
  alias CImg.NIF

  # display object
  #   :handle - Erlang resource object pointing to the CImgDisplay object.
  defstruct handle: nil

  @doc """
  Create display object.
  
  ## Parameters
  
  ## Examples
  
    ```Elixir
    iex> img = CImg.load("sample.jpg")
    iex> display = CImgDisplay(img, "Sample image")
    iex> 
    ```
  """
  def create(cimg, title \\ "", normalization \\ 3, is_fullscreen \\ false, is_closed \\ false) do
    with {:ok, h} <- CImg.NIF.cimgdisplay_u8(cimg, title, normalization, is_fullscreen, is_closed),
      do: %CImgDisplay{handle: h}
  end

  defdelegate wait(cimgdisplay),
    to: NIF, as: :cimgdisplay_wait
  defdelegate wait(cimgdisplay, milliseconds),
    to: NIF, as: :cimgdisplay_wait_time
  defdelegate is_closed(cimgdisplay),
    to: NIF, as: :cimgdisplay_is_closed
  defdelegate button(cimgdisplay),
    to: NIF, as: :cimgdisplay_button
  defdelegate mouse_y(cimgdisplay),
    to: NIF, as: :cimgdisplay_mouse_y
end
