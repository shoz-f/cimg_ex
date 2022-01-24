# Release 0.1.8

## Breaking Changes

## Major Features and Improvements

* supported Nerves/rpi.

## Bug Fixes and Other Changes

* corrected argument checking of `color_mapping/2`.

* replace CImg download script with 'git clone'.

# Release 0.1.7

## Breaking Changes

* deprecated: `to_jpeg/1`, `to_png/1`, `to_flat/2`.<br>
  Use `to_binary/2` instead.

* deprecated: `create_from_bin/6`.<br>
  Use `from_binary/6` instead.

* deprecated: `load_from_memory/1`.<br>
  Use `from_binary/1` instead.

## Major Features and Improvements

* add integrated converter from a image to a binary: `to_binary/2`.

* add color mapping: `color_mapping/2`.

* rename `create_from_bin/6` to `from_binary/6`.

* rename `load_from_memory/1` to `from_binary/1`.

* `to_npy/2` takes various conversion modes as optional parameters.

## Bug Fixes and Other Changes

* correct an axis order of shape in `from_npy/1`, `to_npy/2`.

# Release 0.1.6(Jan 06 2022)

## Breaking Changes

## Major Features and Improvements

* add image convertor `to_jpeg/1`, `to_png/1` (experimental)

## Bug Fixes and Other Changes
