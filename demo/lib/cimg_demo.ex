defmodule CImgDemo do
  alias CImg.Builder

  def demo1(), do: bounce_lens(1)
  def demo2(), do: bounce_lens(2)
  def demo3(), do: profile()

  def bounce_lens(select \\ 1) do
    image  = CImg.load("lena.jpg")
    disp   = CImgDisplay.create(image, "Click a point")

    fisheye = if select == 1 do
      &fisheye/2
    else
      fn res, cnt -> fisheye(res, cnt, fishlens(30)) end
    end

    loop_bounce(0.0, disp, image, fisheye)
  end

  def loop_bounce(alpha, disp, img, fisheye) do
    case CImgDisplay.is_closed(disp) do
      true  -> :ok
      false ->
        {w,h,_,_} = CImg.shape(img)

        CImg.builder(img)
        |> fisheye.(lissajous(alpha, w, h))
        |> CImg.display(disp)

        loop_bounce(alpha+0.02, disp, img, fisheye)
    end
  end

  def lissajous(alpha, w, h) do
    [
      w * (1.0 + 0.9*:math.cos(1.2*alpha))/2.0,
      h * (1.0 + 0.8*:math.sin(3.4*alpha))/2.0,
#      90.0 + 60.0*:math.sin(alpha)
      90.0
    ] |> Enum.map(&trunc/1)
  end

  def fisheye(%Builder{}=screen, [xc, yc, r]) do
    screen
    |> CImg.draw_morph(fishlens(r), xc, yc)
    |> CImg.draw_circle(xc, yc, r, {255,0,255}, 0.3)
    |> CImg.draw_circle(xc, yc, r, {255,0,255}, 1.0, 0xFFFFFFFF)
  end

  def fisheye(%Builder{}=screen, [xc, yc, r], lens) do
    screen
    |> CImg.draw_morph(lens, xc, yc)
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
    screen = CImg.create(500, 400, 1, 3, 0)
    
    main_disp = CImgDisplay.create(image,  "Click a point")
    draw_disp = CImgDisplay.create(screen, "Intensity profile")

    {width,_,_,_} = CImg.shape(image)

    CImgDisplay.wait(main_disp)

    loop_profile(main_disp, fn y ->
      CImg.builder(screen)
        |> CImg.draw_graph(CImg.get_crop(image, 0, y, 0, 0, width-1, y, 0, 0), red,   1.0, 1, 0, 255.0, 0.0)
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
