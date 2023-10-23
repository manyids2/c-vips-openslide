from pathlib import Path
from typing import cast

import numpy as np
import pandas as pd
from dlup import SlideImage
from dlup.data.dataset import TiledROIsSlideImageDataset
from dlup.tiling import TilingMode


def _clip2size(a, size):
    """Clip values from 0 to size boundaries."""
    return np.clip(a, (0, 0), size)


def get_scaled_size(slide_size: tuple[int, int], scaling: float) -> tuple[int, int]:
    """Compute slide image size at specific scaling."""
    size = np.array(slide_size) * scaling
    return cast(tuple[int, int], tuple(size.astype(int)))


def is_valid_region(
    location,
    scaling,
    size,
    slide_size,
) -> bool:
    location = np.asarray(location)
    size = np.asarray(size)
    level_size = np.array(get_scaled_size(slide_size, scaling))
    if (size < 0).any():
        return False
    if ((location < 0) | ((location + size) > level_size)).any():
        return False
    return True


def read_region_request(
    slideimage: SlideImage,
    location,
    scaling,
    size,
):
    owsi = slideimage._wsi
    location = np.asarray(location)
    size = np.asarray(size)
    native_level = owsi.get_best_level_for_downsample(1 / scaling)
    native_level_size = owsi.level_dimensions[native_level]
    native_level_downsample = owsi.level_downsamples[native_level]
    native_scaling = scaling * owsi.level_downsamples[native_level]
    native_location = location / native_scaling
    native_size = size / native_scaling

    # OpenSlide doesn't feature float coordinates to extract a region.
    # We need to extract enough pixels and let PIL do the interpolation.
    # In the borders, the basis functions of other samples contribute to the final value.
    # PIL lanczos uses 3 pixels as support.
    # See pillow: https://git.io/JG0QD
    native_extra_pixels = 3 if native_scaling > 1 else np.ceil(3 / native_scaling)

    # Compute the native location while counting the extra pixels.
    native_location_adapted = np.floor(native_location - native_extra_pixels).astype(
        int
    )
    native_location_adapted = _clip2size(native_location_adapted, native_level_size)

    # Unfortunately openslide requires the location in pixels from level 0.
    level_zero_location_adapted = np.floor(
        native_location_adapted * native_level_downsample
    ).astype(int)
    native_location_adapted = level_zero_location_adapted / native_level_downsample
    native_size_adapted = np.ceil(
        native_location + native_size + native_extra_pixels
    ).astype(int)
    native_size_adapted = (
        _clip2size(native_size_adapted, native_level_size) - native_location_adapted
    )

    # By casting to int we introduce a small error in the right boundary
    # leading to a smaller region which might lead to the target region to
    # overflow from the sampled region.
    native_size_adapted = np.ceil(native_size_adapted).astype(int)

    fractional_coordinates = native_location - native_location_adapted
    request = (
        level_zero_location_adapted[0],
        level_zero_location_adapted[1],
        native_level,
        native_size_adapted[0],
        native_size_adapted[1],
        fractional_coordinates[0],
        fractional_coordinates[1],
        native_size[0],
        native_size[1],
    )

    return request


def read_region(owsi, request):
    (
        location_x,
        location_y,
        level,
        size_x,
        size_y,
        fractional_coordinates_x,
        fractional_coordinates_y,
        native_size_x,
        native_size_y,
    ) = request
    # We extract the region via openslide with the required extra border
    region = owsi.read_region(
        (location_x, location_y),
        level,
        (size_x, size_y),
    )

    fractional_coordinates = np.array(
        [
            fractional_coordinates_x,
            fractional_coordinates_y,
        ]
    )
    native_size = np.array(
        [
            native_size_x,
            native_size_y,
        ]
    )

    # Within this region, there are a bunch of extra pixels, we interpolate to
    # sample the pixel in the right position to retain the right sample weight.
    # We also need to clip to the border, as some readers (e.g mirax) have one
    # pixel less at the border.
    # TODO: This clipping could be in an error in OpenSlide mirax reader, but
    # it's a minor thing for now
    box = (
        *fractional_coordinates,
        *np.clip(
            (fractional_coordinates + native_size),
            a_min=0,
            a_max=np.asarray(region.size),
        ),
    )
    box = cast(tuple[float, float, float, float], box)
    size = cast(tuple[int, int], (size_x, size_y))
    print(box)
    print(size)
    # return region.resize(size, resample=interpolator, box=box)


path = Path("/data/expts/amukundan/el/debug/slides/Aperio/CMU-1.svs")
MPP = 2.0
TILE_SIZE = (256, 256)
TILE_OVERLAP = (0, 0)
TILE_MODE = TilingMode.skip

with SlideImage.from_file_path(path) as wsi:
    slide_mpp = wsi.mpp
    slide_size = wsi.size
    SCALING = wsi.get_scaling(MPP)

    # Other necessary stuff, constant for given mpp
    LEVEL_SIZE = np.array(wsi.get_scaled_size(SCALING))
    BEST_LEVEL = wsi._wsi.get_best_level_for_downsample(1 / SCALING)
    NATIVE_LEVEL_SIZE = wsi._wsi.level_dimensions[BEST_LEVEL]
    NATIVE_LEVEL_DOWNSAMPLE = wsi._wsi.level_downsamples[BEST_LEVEL]
    print("SCALING: ", SCALING)
    print("LEVEL_SIZE: ", LEVEL_SIZE)
    print("BEST_LEVEL: ", BEST_LEVEL)
    print("NATIVE_LEVEL_SIZE: ", NATIVE_LEVEL_SIZE)
    print("NATIVE_LEVEL_DOWNSAMPLE: ", NATIVE_LEVEL_DOWNSAMPLE)

ds = TiledROIsSlideImageDataset.from_standard_tiling(
    path=path,
    mpp=MPP,
    tile_size=TILE_SIZE,
    tile_overlap=TILE_OVERLAP,
    mode=TILE_MODE,
)


location = (6912, 5376)
scaling = SCALING
size = TILE_SIZE
if is_valid_region(location, scaling, size, slide_size):
    request = read_region_request(ds.slide_image, location, scaling, size)
    print(request)

    read_region(ds.slide_image, request)
