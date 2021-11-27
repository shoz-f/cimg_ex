defmodule CImgDemo do
  def demo1(), do: bounce_lens(1)
  def demo2(), do: bounce_lens(2)
  def demo3(), do: profile()

  def bounce_lens(select \\ 1) do
    image  = CImg.load("lena.jpg")
    screen = CImg.create(512, 512, 1, 3, 0)
    disp   = CImgDisplay.create(image, "Click a point")

    fisheye = if select == 1 do
      &fisheye/3
    else
      fn res, img, cnt -> fisheye(res, img, cnt, fishlens(30)) end
    end

    loop_bounce(0.0, disp, screen, image, fisheye)
  end

  def loop_bounce(alpha, disp, res, img, fisheye) do
    case CImgDisplay.is_closed(disp) do
      true  -> :ok
      false ->
        {w,h,_,_} = CImg.shape(img)
        CImg.assign(res, img)
        |> fisheye.(img, lissajous(alpha, w, h))
        |> CImg.display(disp)

        loop_bounce(alpha+0.02, disp, res, img, fisheye)
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


  def profile() do
    red   = {255, 0, 0}
    green = {0, 255, 0}
    blue  = {0, 0, 255}

    image  = CImg.load("lena.jpg") |> CImg.blur(2.5)
    canvas = CImg.create(500, 400, 1, 3, 0)
    
    main_disp = CImgDisplay.create(image, "Click a point")
    draw_disp = CImgDisplay.create(canvas, "Intensity profile")

    {width,_,_,_} = CImg.shape(image)

    CImgDisplay.wait(main_disp)

    loop_profile(main_disp, fn y ->
      canvas
        |> CImg.fill(0)
        |> CImg.draw_graph(CImg.get_crop(image, 0, y, 0, 0, width-1, y, 0, 0), red,   1, 1, 0, 255, 0)
        |> CImg.draw_graph(CImg.get_crop(image, 0, y, 0, 1, width-1, y, 0, 1), green, 1.0, 1, 0, 255.0, 0.0)
        |> CImg.draw_graph(CImg.get_crop(image, 0, y, 0, 2, width-1, y, 0, 2), blue,  1.0, 1, 0, 255.0, 0.0)
        |> CImg.display(draw_disp)
    end)
  end
  
  defp loop_profile(disp, draw_profile) do
    case CImgDisplay.is_closed(disp) do
      true  -> :ok
      false ->
        draw_profile.(CImgDisplay.mouse_y(disp))
        CImgDisplay.wait(disp, 50)
        loop_profile(disp, draw_profile)
    end
  end
end
