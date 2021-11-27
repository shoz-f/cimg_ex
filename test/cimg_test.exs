defmodule CImgTest do
  use ExUnit.Case
  #doctest CImg

  test "create" do
    img = CImg.load("test/IMG_9458.jpg")
    assert %CImg{} = img
    assert {2448, 3264, _, _} = CImg.shape(img)
    assert :ok = CImg.save(img, "test/IMG_XXXX.jpg")
  end

  test "resize(416, 416)" do
    img = CImg.load("test/IMG_9458.jpg")
    assert %CImg{} = img

    img
    |> CImg.resize({416, 416})
    |> CImg.mirror(:y)
    |> CImg.make_gray()
    |> CImg.draw_box(0.2, 0.3, 0.4, 0.6, {255, 0, 0})
    |> CImg.save("test/416x416.jpg")
  end
end
