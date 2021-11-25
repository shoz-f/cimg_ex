# CImg
Light-weight image processing module in Elixir with CImg. This module aims to create auxiliary routines for Deep Learning.

Note: It still has a few image processing functions currentrly.

## Platform
It has been confirmed to work in the following OS environment.

- Windows MSYS2/MinGW64
- WSL2/Ubuntu 20.04

## Requirements
The following libraries are required to display images on the PC screen.

- GDI32 on Windows
- X11 on Linux

## Installation
Add following dependency to your `mix.exs`.

```elixir
def deps do
  [
    {:cimg, "~> 0.1.0"}
  ]
end
```

and install dependencies:

```shell
$ mix deps.get
$ mix deps.compile
```

## Demo
There is a simple program in demo directory. You can do it by following the steps below.

```shell
$ cd demo
$ mix deps.get
$ mix run -e "CImgDemo.demo(1)"
```

Close the appaired window, and stop the demo program.

Another demo:

```shell
$ mix run -e "CImgDemo.demo(2)"
```

## License
Cimg is licensed under the Apache License Version 2.0.

#### -- license overview of included 3rd party libraries --
- The "CImg" Library is licensed under the CeCILL-C/CeCILL.
- The "stb" - single-file public domain libraries for C/C++ - is public domain or MIT licensed.
