# Changelog

## Release 0.1.14

  * Major Features and Improvements
    * added "fill_rect" and "fill_circle".
    * added CImg.Util module including 'aspect' and 'rand_palette'.

## Release 0.1.13

  * Major Features and Improvements
    * added Gaussian distribution transformation to `to_binary/e2` and `from_binary/6`.

  * Bug Fixes and Other Changes
    * fixed a bug of inverse range conversion of from_binary/6.

## Release 0.1.12

  * Major Features and Improvements
    * add image blending operation `blend/3`.

  * Bug Fixes and Other Changes
    * revised `Makefile` for possible to build for Nerves devices on MacOS. (PR#1)

## Release 0.1.11

  * Major Features and Improvements
    * add convertion options to `from_binary/6`.

  * Bug Fixes and Other Changes
    * correct document.

## Release 0.1.10

  * Breaking Changes
    * remove deprecated functions. see `Release 0.1.7` changelog.
    * remove Builder.from_binary/1, Builder.resize/4. replaced by session.

  * Major Features and Improvements
    * add session execution mode.
    * add CImg.display_kino/2.

  * Bug Fixes and Other Changes
    * fixed a bug that did not release image resources.

## Release 0.1.9

  * Major Features and Improvements
    * add draw_text/8.
    * add Builder.from_binary/1, enable Builder.resize/4 (experimental)

## Release 0.1.8

  * Major Features and Improvements
    * supported Nerves/rpi.
    * `resize/4` accepts resize scale instead {x, y}.

  * Bug Fixes and Other Changes
    * corrected argument checking of `color_mapping/2`.
    * replace CImg download script with 'git clone'.

## Release 0.1.7

  * Breaking Changes
    * deprecated: `to_jpeg/1`, `to_png/1`, `to_flat/2`. Use `to_binary/2` instead.
    * deprecated: `create_from_bin/6`. Use `from_binary/6` instead.
    * deprecated: `load_from_memory/1`. Use `from_binary/1` instead.

  * Major Features and Improvements
    * add integrated converter from a image to a binary: `to_binary/2`.
    * add color mapping: `color_mapping/2`.
    * rename `create_from_bin/6` to `from_binary/6`.
    * rename `load_from_memory/1` to `from_binary/1`.
    * `to_npy/2` takes various conversion modes as optional parameters.

  * Bug Fixes and Other Changes
    * correct an axis order of shape in `from_npy/1`, `to_npy/2`.

## Release 0.1.6 (Jan 06 2022)

  * Major Features and Improvements
    * add image convertor `to_jpeg/1`, `to_png/1` (experimental)
