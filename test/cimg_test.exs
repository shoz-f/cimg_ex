defmodule CImgTest do
  use ExUnit.Case
  doctest CImg

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
    |> CImg.get_gray()
    |> CImg.draw_box(0.2, 0.3, 0.4, 0.6, {255, 0, 0})
    |> CImg.save("test/416x416.jpg")
  end
  
  test "tutorial" do
    red   = {255, 0, 0}
    green = {0, 255, 0}
    blue  = {0, 0, 255}

    image = CImg.load("test/lena.jpg") |> CImg.blur(2.5)
    visu  = CImg.create(500, 400, 1, 3, 0)
    
    main_disp = CImgDisplay.create(image, "Click a point")
    draw_disp = CImgDisplay.create(visu, "Intensity profile")

    {width,_,_,_} = CImg.shape(image)
    

    CImgDisplay.wait(main_disp)

    y = CImgDisplay.mouse_y(main_disp)

    visu
    |> CImg.fill(0)
    |> CImg.draw_graph(CImg.get_crop(image, 0, y, 0, 0, width-1, y, 0, 0), red,   1, 1, 0, 255, 0)
    |> CImg.draw_graph(CImg.get_crop(image, 0, y, 0, 1, width-1, y, 0, 1), green, 1.0, 1, 0, 255.0, 0.0)
    |> CImg.draw_graph(CImg.get_crop(image, 0, y, 0, 2, width-1, y, 0, 2), blue,  1.0, 1, 0, 255.0, 0.0)
    |> CImg.display(draw_disp)
    
    CImgDisplay.wait(main_disp, 2000)
  end
end
