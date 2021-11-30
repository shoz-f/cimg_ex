defmodule Cimg.MixProject do
  use Mix.Project

  def project do
    [
      app: :cimg,
      version: "0.1.4",
      elixir: "~> 1.10",
      start_permanent: Mix.env() == :prod,
      make_executable: "make",
      make_clean: ["clean"],
      compilers: [:elixir_make] ++ Mix.compilers(),
      deps: deps(),
      
      description: description(),
      package: package(),
      #name: "cimg",
      source_url: "https://github.com/shoz-f/cimg_ex.git"
    ]
  end

  # Run "mix help compile.app" to learn about applications.
  def application do
    [
      extra_applications: [:logger]
    ]
  end

  # Run "mix help deps" to learn about dependencies.
  defp deps do
    [
      {:elixir_make, "~> 0.6.2", runtime: false},
      {:ex_doc, "~> 0.24", only: :dev, runtime: false},
    ]
  end

  defp description() do
    "Light-weight image processing module in Elixir with CImg."
  end

  defp package() do
    [
      name: "cimg",
      licenses: ["Apache-2.0"],
      links: %{"GitHub" => "https://github.com/shoz-f/cimg_ex.git"}
    ]
  end
end
