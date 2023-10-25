"""Return a region at a specific scaling level of the pyramid.

A typical slide is made of several levels at different mpps.
In normal cirmustances, it's not possible to retrieve an image of
intermediate mpp between these levels. This method takes care of
sumbsampling the closest high resolution level to extract a target
region via interpolation.

Once the best layer is selected, a native resolution region
is extracted, with enough padding to include the samples necessary to downsample
the final region (considering LANCZOS interpolation method basis functions).

The steps are approximately the following:

1. Map the region that we want to extract to the below layer.
2. Add some extra values (left and right) to the native region we want to extract
   to take into account the interpolation samples at the border ("native_extra_pixels").
3. Map the location to the level0 coordinates, floor it to add extra information
   on the left (level_zero_location_adapted).
4. Re-map the integral level-0 location to the native_level.
5. Compute the right bound of the region adding the native_size and extra pixels (native_size_adapted).
   The size is also clipped so that any extra pixel will fit within the native level.
6. Since the native_size_adapted needs to be passed to openslide and has to be an integer, we ceil it
   to avoid problems with possible overflows of the right boundary of the target region being greater
   than the right boundary of the sample region
   (native_location + native_size > native_size_adapted + native_location_adapted).
7. Crop the target region from within the sampled region by computing the relative
   coordinates (fractional_coordinates).

Parameters
----------
location :
    Location from the top left (x, y) in pixel coordinates given at the requested scaling.
scaling :
    The scaling to be applied compared to level 0.
size :
    Region size of the resulting region.

Returns
-------
PIL.Image
    The extract region.

Examples
--------
The locations are defined at the requested scaling (with respect to level 0), so if we want to extract at
location ``(location_x, location_y)`` of a scaling 0.5 (with respect to level 0), and have
resulting tile size of ``(tile_size, tile_size)`` with a scaling factor of 0.5, we can use:
>>>  wsi.read_region(location=(coordinate_x, coordinate_y), scaling=0.5, size=(tile_size, tile_size))
"""

from typing import cast

import numpy as np


def _clip2size(a, size):
    """Clip values from 0 to size boundaries."""
    return np.clip(a, (0, 0), size)


def is_valid_region(
    location,
    scaling,
    size,
    slide_size,
):
    location = np.asarray(location)  # float
    size = np.asarray(size)  # float
    level_size = np.array(get_scaled_size(scaling, slide_size))  # tuple[int, int]
    if (size < 0).any():
        raise ValueError("Size values must be greater than zero.")

    if ((location < 0) | ((location + size) > level_size)).any():
        raise ValueError(
            (
                f"Requested region is outside level boundaries:\n"
                f"                  (location < 0): ({location} < 0)\n"
                f"  (location + size) > level_size: ({location} + {size}) > {level_size}\n"
                f"                                = ({location + size}) > {level_size}\n"
            )
        )


def read_request(
    # Dlup read_region args
    location,  # : tuple[int, int],
    scaling,  # : float,
    size,  # : tuple[int, int],
    # Get the rest from openslide
    best_level_for_downsample: int,
    level_dimensions: list[tuple[int, int]],
    level_downsamples: list[tuple[float, float]],
):
    # NOTE: Assuming `is_valid_region(...) == True`
    location = np.asarray(location)  # float
    size = np.asarray(size)  # float
    print(f"inputs:\n" f"  location  : {location}\n" f"  size      : {size}\n")

    # NOTE: Get from slide
    # native_level = get_best_level_for_downsample(1 / scaling)  # int
    native_level = best_level_for_downsample

    # NOTE: Immediately calculate native level coords
    native_level_size = level_dimensions[native_level]  # tuple[int, int]
    native_level_downsample = level_downsamples[native_level]  # float
    native_scaling = scaling * level_downsamples[native_level]  # float
    native_location = location / native_scaling  # tuple[float, float]
    native_size = size / native_scaling  # tuple[float, float]
    print(
        f"native:\n"
        f"  native_level            : {native_level}\n"
        f"  native_level_size       : {native_level_size}\n"
        f"  native_level_downsample : {native_level_downsample}\n"
        f"  native_location         : {native_location}\n"
        f"  native_scaling          : {native_scaling}\n"
        f"  native_size             : {native_size}\n"
    )

    # OpenSlide doesn't feature float coordinates to extract a region.
    # We need to extract enough pixels and let PIL do the interpolation.
    # In the borders, the basis functions of other samples contribute to the final value.
    # PIL lanczos uses 3 pixels as support.
    # See pillow: https://git.io/JG0QD ( link to code of resize )
    # NOTE: Compute adaptation for openslide
    # float
    native_extra_pixels = 3 if native_scaling > 1 else np.ceil(3 / native_scaling)
    print(f"native:\n" f"  native_extra_pixels: {native_extra_pixels}\n")

    # Compute the native location while counting the extra pixels.
    # tuple[int, int]
    native_location_adapted = np.floor(native_location - native_extra_pixels).astype(
        int
    )
    # tuple[int, int]
    native_location_adapted = _clip2size(native_location_adapted, native_level_size)
    print(f"native:\n" f"  native_location_adapted (before): {native_location_adapted}")

    # Unfortunately openslide requires the location in pixels from level 0.
    # tuple[int, int]
    level_zero_location_adapted = np.floor(
        native_location_adapted * native_level_downsample
    ).astype(int)
    # tuple[float, float]
    native_location_adapted = level_zero_location_adapted / native_level_downsample
    # tuple[int, int]
    native_size_adapted = np.ceil(
        native_location + native_size + native_extra_pixels
    ).astype(int)
    # tuple[float, float]
    native_size_adapted = (
        _clip2size(native_size_adapted, native_level_size) - native_location_adapted
    )

    print(
        f"native:\n"
        f"  level_zero_location_adapted : {level_zero_location_adapted}\n"
        f"  native_location_adapted     : {native_location_adapted}\n"
        f"  native_size_adapted         : {native_size_adapted}\n"
    )

    # By casting to int we introduce a small error in the right boundary leading
    # to a smaller region which might lead to the target region to overflow from
    # the sampled region.
    # tuple[int, int]
    native_size_adapted = np.ceil(native_size_adapted).astype(int)
    print(f"native:\n" f"  native_size_adapted: {native_size_adapted}\n")

    # Within this region, there are a bunch of extra pixels, we interpolate to sample
    # the pixel in the right position to retain the right sample weight.
    # We also need to clip to the border, as some readers (e.g mirax) have one pixel less at the border.
    # tuple[float, float]
    fractional_coordinates = native_location - native_location_adapted

    print(
        f"Request:\n"
        f"  location: {(level_zero_location_adapted[0], level_zero_location_adapted[1])}\n"
        f"  level   : {native_level}\n"
        f"  size    : {(native_size_adapted[0], native_size_adapted[1])}\n"
        f"  Native  :\n"
        f"    location: {fractional_coordinates}\n"
        f"    level   : {native_size}\n"
    )


# def read_region():
# # We extract the region via openslide with the required extra border
# # region = owsi.read_region(
#     (
#         level_zero_location_adapted[0],
#         level_zero_location_adapted[1],
#     ),  # tuple[int, int]
#     native_level,  # int
#     (native_size_adapted[0], native_size_adapted[1]),  # tuple[int, int]
# )
# print(f"Region:\n" f"  size: {region.size}\n")
#
#     # TODO: This clipping could be in an error in OpenSlide mirax reader, but it's a minor thing for now
#     box = (
#         *fractional_coordinates,
#         *np.clip(
#             (fractional_coordinates + native_size),
#             a_min=0,
#             a_max=np.asarray(region.size),
#         ),
#     )
#     box = cast(tuple[float, float, float, float], box)
#     size = cast(tuple[int, int], size)
#     return region.resize(size, resample=self._interpolator, box=box)


def get_best_level_for_downsample(inv_scaling):
    scaling = 1 / inv_scaling
    print(scaling)
    # TODO:
    return 1


def get_scaled_size(scaling: float, slide_size: tuple[int, int]) -> tuple[int, int]:
    """Compute slide image size at specific scaling."""
    return cast(tuple[int, int], tuple((np.array(slide_size) * scaling).astype(int)))
