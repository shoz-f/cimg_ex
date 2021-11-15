defmodule CImgDemo do
  def demo2 do
    image  = CImg.load("test/lena.jpg")
    screen = CImg.create(512, 512, 1, 3, 0)
    disp   = CImgDisplay.create(image, "Click a point")

    demo2_loop(0.0, disp, screen, image, fishlens(90))
  end

  def demo2_loop(alpha, disp, res, img, lens) do
    case CImgDisplay.is_closed(disp) do
      true  -> :ok
      false ->
        {w,h,_,_} = img.shape
        CImg.assign(res, img)
        #|> fisheye2(img, lissajous(alpha, w, h))
        |> fisheye3(img, lissajous(alpha, w, h), lens)
        |> CImg.display(disp)

        demo2_loop(alpha+0.02, disp, res, img, lens)
    end
  end

  def lissajous(alpha, w, h) do
    [
      w * (1.0 + 0.9*:math.cos(1.2*alpha))/2.0,
      h * (1.0 + 0.8*:math.sin(3.4*alpha))/2.0,
      #90.0 + 60.0*:math.sin(alpha)
      90.0
    ] |> Enum.map(&trunc/1)
  end

  def fisheye2(res, img, [xc, yc, r]) do
    res
    |> CImg.transfer(img, fishlens(r), xc, yc)
    |> CImg.draw_circle(xc, yc, r, {255,0,255}, 0.3)
    |> CImg.draw_circle(xc, yc, r, {255,0,255}, 1.0, 0xFFFFFFFF)
  end

  def fisheye3(res, img, [xc, yc, r], lens) do
    res
    |> CImg.transfer(img, lens, xc, yc)
    |> CImg.draw_circle(xc, yc, r, {255,0,255}, 0.3)
    |> CImg.draw_circle(xc, yc, r, {255,0,255}, 1.0, 0xFFFFFFFF)
  end

  def fishlens(r) do
    for y <- -r..r, x <- -r..r, (rr = :math.sqrt(x*x + y*y)/r) < 1.0 do
      {[x,y],[trunc(rr*x),trunc(rr*y)]}
    end
  end
end
