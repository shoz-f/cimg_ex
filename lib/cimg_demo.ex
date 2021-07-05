defmodule CImgDemo do
  def demo2 do
    image  = CImg.create("test/lena.jpg")
    screen = CImg.create(512, 512, 1, 3, 0)
    disp   = CImgDisplay.create(image, "Click a point")

    demo2_loop(0.0, disp, screen, image)
  end

  def demo2_loop(alpha, disp, res, img) do
    case CImgDisplay.is_closed(disp) do
      true  -> :ok
      false ->
        {w,h,_,_} = img.shape
        CImg.assign(res, img)
        |> fisheye2(img, lissajous(alpha, w, h))
        |> CImg.display(disp)

        demo2_loop(alpha+0.02, disp, res, img)
    end
  end

  def lissajous(alpha, w, h) do
    [
      w * (1.0 + 0.9*:math.cos(1.2*alpha))/2.0,
      h * (1.0 + 0.8*:math.sin(3.4*alpha))/2.0,
      #90.0 + 60.0*:math.sin(alpha)
      60.0
    ] |> Enum.map(&trunc/1)
  end

  def fisheye(res, img, [xc, yc, r]) do
    {w,h,_,_} = CImg.shape(img)
    Enum.each(fishlens(r, [xc, yc]), fn {[x,y],[s,t]} ->
      if x >= 0 && x <= w-1 && y >= 0 && y <= h-1 do
        CImg.get(img, s, t, 0) |> CImg.set(res, x, y, 0)
        CImg.get(img, s, t, 1) |> CImg.set(res, x, y, 1)
        CImg.get(img, s, t, 2) |> CImg.set(res, x, y, 2)
      end
    end)
    CImg.draw_circle(res, xc, yc, r, {255,0,255}, 0.3)
    |> CImg.draw_circle(xc, yc, r, {255,0,255}, 1.0, 0xFFFFFFFF)
  end

  def fisheye2(res, img, [xc, yc, r]) do
    res
    |> CImg.transfer(img, fishlens(r, [xc, yc]))
    |> CImg.draw_circle(xc, yc, r, {255,0,255}, 0.3)
    |> CImg.draw_circle(xc, yc, r, {255,0,255}, 1.0, 0xFFFFFFFF)
  end

  def fishlens(r), do: fishlens(r, [0, 0])
  def fishlens(r, [xc, yc]) do
    for y <- -r..r, x <- -r..r, (rr = :math.sqrt(x*x + y*y)/r) < 1.0 do
      {[xc+x,yc+y],[xc+trunc(rr*x),yc+trunc(rr*y)]}
    end
  end
end
