defmodule CImgDemo do
  def demo(select \\ 1) do
    image  = CImg.load("lena.jpg")
    screen = CImg.create(512, 512, 1, 3, 0)
    disp   = CImgDisplay.create(image, "Click a point")

    fisheye = if select == 1 do
      &fisheye/3
    else
      fn res, img, cnt -> fisheye(res, img, cnt, fishlens(30)) end
    end

    do_loop(0.0, disp, screen, image, fisheye)
  end

  def do_loop(alpha, disp, res, img, fisheye) do
    case CImgDisplay.is_closed(disp) do
      true  -> :ok
      false ->
        {w,h,_,_} = img.shape
        CImg.assign(res, img)
        |> fisheye.(img, lissajous(alpha, w, h))
        |> CImg.display(disp)

        do_loop(alpha+0.02, disp, res, img, fisheye)
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

  def fisheye(res, img, [xc, yc, r]) do
    res
    |> CImg.transfer(img, fishlens(r), xc, yc)
    |> CImg.draw_circle(xc, yc, r, {255,0,255}, 0.3)
    |> CImg.draw_circle(xc, yc, r, {255,0,255}, 1.0, 0xFFFFFFFF)
  end

  def fisheye(res, img, [xc, yc, r], lens) do
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
