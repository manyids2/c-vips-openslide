#include "slide.h"

int length_associated_images(openslide_t *osr) {
  int count = 0;
  const char *const *associated_image_names =
      openslide_get_associated_image_names(osr);
  while (*associated_image_names) {
    count += 1;
    associated_image_names++;
  };
  return count;
}

/* Return a region at a specific scaling level of the pyramid.
 *
 * A typical slide is made of several levels at different mpps.
 * In normal cirmustances, it's not possible to retrieve an image of
 * intermediate mpp between these levels. This method takes care of
 * sumbsampling the closest high resolution level to extract a target
 * region via interpolation.
 *
 * Once the best layer is selected, a native resolution region
 * is extracted, with enough padding to include the samples necessary to
 * downsample the final region (considering LANCZOS interpolation method
 * basis functions).
 *
 * The steps are approximately the following:
 *
 * 1. Map the region that we want to extract to the below layer.
 * 2. Add some extra values (left and right) to the native region we want to
 * extract
 *   to take into account the interpolation samples at the border
 *   ("native_extra_pixels").
 * 3. Map the location to the level0 coordinates, floor it to add extra
 * information
 *   on the left (level_zero_location_adapted).
 * 4. Re-map the integral level-0 location to the native_level.
 * 5. Compute the right bound of the region adding the native_size and extra
 * pixels (native_size_adapted).
 *   The size is also clipped so that any extra pixel will fit within the
 *   native level.
 * 6. Since the native_size_adapted needs to be passed to openslide and has
 * to be an integer, we ceil it
 *   to avoid problems with possible overflows of the right boundary of the
 *   target region being greater than the right boundary of the sample
 *   region (native_location + native_size > native_size_adapted +
 *   native_location_adapted).
 * 7. Crop the target region from within the sampled region by computing the
 * relative
 *   coordinates (fractional_coordinates).
 */
int read_region_request(request_t *request, ipos_t location, double scaling,
                        ipos_t size, openslide_t *osr,
                        level_props_t *level_props) {
  // -- Step 1 : Map region ---
  ipos_t level_size = get_scaled_size(level_props->slide_size, scaling);

  // Size values must be greater than zero.
  if ((level_size.x < 0) | (level_size.y < 0)) {
    return -1;
  }

  // Requested region is outside level boundaries.
  if ((location.x < 0) | (location.y < 0) |
      (location.x + size.x > level_size.x) |
      (location.y + size.y > level_size.y)) {
    return -1;
  }

  // Map to native level
  int native_level = openslide_get_best_level_for_downsample(osr, 1 / scaling);
  ipos_t native_level_size = level_props->level_dimensions[native_level];

  // Henceforth almost everything is double until we cast to int64
  double native_level_downsample = level_props->level_downsamples[native_level];
  double native_scaling = scaling * native_level_downsample;
  dpos_t dlocation = {.x = location.x, .y = location.y};
  dpos_t dsize = {.x = size.x, .y = size.y};
  dpos_t native_location = _div(dlocation, native_scaling);
  dpos_t native_size = _div(dsize, native_scaling);

  // -- Step 2 : Compute extra pixels for sampling ---
  // OpenSlide doesn't feature float coordinates to extract a region.
  // We need to extract enough pixels and let PIL do the interpolation.
  // In the borders, the basis functions of other samples contribute to
  // the final value.
  // PIL lanczos uses 3 pixels as support. See pillow: https://git.io/JG0QD
  double native_extra_pixels;
  if (native_scaling > 1) {
    native_extra_pixels = 3.0;
  } else {
    native_extra_pixels = ceil(3 / native_scaling);
  }

  // Compute the native location while counting the extra pixels.
  dpos_t native_location_adapted =
      _floor(_sub(native_location, native_extra_pixels));
  dpos_t native_size_adapted =
      _clip2size(native_location_adapted, native_level_size);

  // Unfortunately openslide requires the location in pixels from level 0.
  dpos_t level_zero_location_adapted =
      _floor(_mul(native_location_adapted, native_level_downsample));

  // Recompute native_size_adapted, native_location_adapted
  native_location_adapted =
      _div(level_zero_location_adapted, native_level_downsample);
  native_size_adapted =
      _ceil(_add(_addv(native_location, native_size), native_extra_pixels));
  native_size_adapted = _clip2size(native_size_adapted, native_level_size);
  native_size_adapted = _subv(native_size_adapted, native_location_adapted);

  // By casting to int we introduce a small error in the right boundary
  // leading # to a smaller region which might lead to the target region to
  // overflow from the sampled # region.
  ipos_t ilevel_zero_location_adapted = {
      .x = level_zero_location_adapted.x,
      .y = level_zero_location_adapted.y,
  };
  ipos_t inative_size_adapted = {
      .x = native_size_adapted.x,
      .y = native_size_adapted.y,
  };

  // For read_region
  request->location = ilevel_zero_location_adapted;
  request->level = native_level;
  request->size = inative_size_adapted;

  // For post processing
  request->native.fractional_coordinates =
      _subv(native_location, native_location_adapted);
  request->native.native_size = native_size;

  return 0;
}

// int read_region(image_t *region, openslide_t *osr, request_t request,
//                 Resampling resampling) {
//   // We extract the region via openslide with the required extra border
//   openslide_read_region(osr, (uint32_t *)region->data, request.location.x,
//                         request.location.y, request.level, request.size.x,
//                         request.size.y);
//
//   // TODO: Read size of returned region
//   ipos_t region_size = {.x = 10, .y = 10};
//
//   // # Within this region, there are a bunch of extra pixels, we interpolate
//   // to sample # the pixel in the right position to retain the right sample
//   // weight. # We also need to clip to the border, as some readers (e.g
//   // mirax) have one pixel less at the border.
//
//   // # TODO: This clipping could be in an error in OpenSlide mirax reader,
//   // but it's a minor thing for now
//   dpos_t fractional_coordinates = request.native.fractional_coordinates;
//   dpos_t native_size = request.native.native_size;
//   dpos_t clipped_bottom_right =
//       _clip2size(_addv(fractional_coordinates, native_size), region_size);
//   dbox_t box = {
//       .x1 = fractional_coordinates.x,
//       .y1 = fractional_coordinates.y,
//       .x2 = clipped_bottom_right.x,
//       .y2 = clipped_bottom_right.y,
//   };
//   ipos_t size = request.size;
//
//   // TODO: Finally, resize and return
//   // return region.resize(size, resample=resampling, box=box)
//
//   return 0;
// }
