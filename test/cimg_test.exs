defmodule CImgTest do
  use ExUnit.Case
  #doctest CImg

  test "create" do
    img = CImg.load("test/IMG_9458.jpg")
    assert %CImg{} = img
    assert {2448, 3264, _, _} = CImg.shape(img)
    assert :ok = CImg.save(img, "test/IMG_XXXX.jpg")
  end

#  test "resize(416, 416)" do
#    img = CImg.load("test/IMG_9458.jpg")
#    assert %CImg{} = img
#
#    img
#    |> CImg.resize({416, 416})
#    |> CImg.mirror(:y)
#    |> CImg.gray()
#    |> CImg.builder()
#    |> CImg.draw_rect(0.2, 0.3, 0.4, 0.6, {255, 0, 0})
#    |> CImg.runit()
#    |> CImg.save("test/416x416.jpg")
#  end

  test "CImg functions" do
    img = CImg.load("test/IMG_9458.jpg")
    
    resize = CImg.resize(img, {416,416})
    CImg.save(resize, "resize.jpg")
    
    invert = CImg.invert(resize)
    CImg.save(invert, "invert.jpg")
    
    gray = CImg.gray(resize)
    CImg.save(gray, "gray.jpg")
    
    thresh = CImg.threshold(gray, 100)
    CImg.save(thresh, "thresh.jpg")
    
    mapping = CImg.color_mapping(thresh, :lines)
    CImg.save(mapping, "mapping.jpg")
    
    blur = CImg.blur(resize, 5)
    CImg.save(blur, "blur.jpg")
    
    mirror = CImg.mirror(resize, :y)
    CImg.save(mirror, "mirror.jpg")
    
    transp = CImg.transpose(resize)
    CImg.save(transp, "transp.jpg")
    
    CImg.builder(img)
    |> CImg.resize({320,320})
    |> CImg.invert()
    |> CImg.transpose()
    |> CImg.run()
    |> CImg.save("rit.jpg")

    CImg.save(img, "original.jpg")
  end
end
