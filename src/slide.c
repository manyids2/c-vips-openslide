#include "slide.h"
#include "properties.h"
#include <openslide/openslide.h>
#include <stdio.h>
#include <string.h>

oslide_t open_oslide(char *path) {
  openslide_t *osr = openslide_open(path);

  // Allocate for path?
  // oslide.path = malloc((strlen(path) + 1) * sizeof(char));

  // Initialize primitives
  oslide_t oslide = {.path = path,
                     .osr = osr,
                     .slide_props =
                         {
                             .mpp = get_mpp(osr),
                             .magnification = get_magnification(osr),
                             .size = get_size(osr),
                             .spacings = get_spacings(osr),
                             .offset = get_offset(osr),
                             .bounds = get_bounds(osr),
                         },
                     .level_props = {
                         .level_count = openslide_get_level_count(osr),
                     }};

  // Need to malloc for dynamic arrays
  int level_count = openslide_get_level_count(osr);
  oslide.level_props.level_count = level_count;

  // Downsamples
  oslide.level_props.level_downsamples = malloc(level_count * sizeof(double));
  get_level_downsamples(osr, level_count, oslide.level_props.level_downsamples);

  // Dimensions
  oslide.level_props.level_dimensions = malloc(level_count * sizeof(ipos_t));
  get_level_dimensions(osr, level_count, oslide.level_props.level_dimensions);

  return oslide;
}

void print_slide(oslide_t *oslide) {
  printf("mpp     : %f\n", oslide->slide_props.mpp);
  printf("spacing : %f, %f\n", oslide->slide_props.spacings.x,
         oslide->slide_props.spacings.y);
  printf("size    : %7ld, %7ld\n", oslide->slide_props.size.x,
         oslide->slide_props.size.y);
  printf("offset  : %7ld, %7ld\n", oslide->slide_props.offset.x,
         oslide->slide_props.offset.y);
  printf("bounds  : %7ld, %7ld\n", oslide->slide_props.bounds.x,
         oslide->slide_props.bounds.y);
  printf("level_count : %d\n", oslide->level_props.level_count);
  printf("level_downsamples: \n");
  for (int level = 0; level < oslide->level_props.level_count; level++) {
    printf("  %2d: %f\n", level, oslide->level_props.level_downsamples[level]);
  }
  printf("level_dimensions : \n");
  for (int level = 0; level < oslide->level_props.level_count; level++) {
    printf("  %2d: %7ld,%7ld\n", level,
           oslide->level_props.level_dimensions[level].x,
           oslide->level_props.level_dimensions[level].y);
  }
}

void close_oslide(oslide_t *oslide) {
  // Downsamples
  if (oslide->level_props.level_downsamples) {
    free(oslide->level_props.level_downsamples);
  }
  // Dimensions
  if (oslide->level_props.level_dimensions) {
    free(oslide->level_props.level_dimensions);
  }
  openslide_close(oslide->osr);
}

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

int get_thumbnail(openslide_t *osr, image_t *thumbnail, AssociatedImage name) {
  const char *const *associated_image_names =
      openslide_get_associated_image_names(osr);

  const char *iname = stringFromAssociatedImage(name);
  while (*associated_image_names) {
    int64_t w, h;
    const char *name = *associated_image_names;
    if (!strcmp(name, iname)) {
      // Get size of thumbnail
      openslide_get_associated_image_dimensions(osr, name, &w, &h);

      // Set size, allocate data
      // NOTE: Remember to free
      thumbnail->width = w;
      thumbnail->height = h;
      thumbnail->bands = 4; // ARGB
      thumbnail->data = malloc(w * h * sizeof(uint32_t));

      // Read thumbnail
      openslide_read_associated_image(osr, name, (uint32_t *)thumbnail->data);

      // No error
      return 0;
    }
    associated_image_names++;
  }
  // Could not find image
  return 1;
}

void get_level_downsamples(openslide_t *osr, int level_count,
                           double *level_downsamples) {
  for (int level = 0; level < level_count; level++) {
    level_downsamples[level] = openslide_get_level_downsample(osr, level);
  }
}

void get_level_dimensions(openslide_t *osr, int level_count,
                          ipos_t *level_dimensions) {
  for (int level = 0; level < level_count; level++) {
    int64_t h, w;
    openslide_get_level_dimensions(osr, level, &w, &h);
    ipos_t dimensions = {.x = w, .y = h};
    level_dimensions[level] = dimensions;
  }
}

ipos_t get_size(openslide_t *osr) {
  int64_t h, w;
  openslide_get_level0_dimensions(osr, &w, &h);
  ipos_t size = {.x = w, .y = h};
  return size;
}

double get_mpp(openslide_t *osr) {
  double mpp;
  const char *c_mpp = openslide_get_property_value(osr, PROPERTY_NAME_MPP_X);
  // Set to zero if not found
  if (!c_mpp) {
    mpp = 0.0;
  } else {
    mpp = atof(c_mpp);
  }
  return mpp;
}

double get_magnification(openslide_t *osr) {
  double magnification;
  const char *c_magnification =
      openslide_get_property_value(osr, PROPERTY_NAME_OBJECTIVE_POWER);
  // Set to zero if not found
  if (!c_magnification) {
    magnification = 0.0;
  } else {
    magnification = atof(c_magnification);
  }
  return magnification;
}

dpos_t get_spacings(openslide_t *osr) {
  double mpp_x;
  const char *c_mpp_x = openslide_get_property_value(osr, PROPERTY_NAME_MPP_X);
  // Set to zero if not found
  if (!c_mpp_x) {
    mpp_x = 0.0;
  } else {
    mpp_x = atof(c_mpp_x);
  }

  double mpp_y;
  const char *c_mpp_y = openslide_get_property_value(osr, PROPERTY_NAME_MPP_Y);
  // Set to zero if not found
  if (!c_mpp_y) {
    mpp_y = 0.0;
  } else {
    mpp_y = atof(c_mpp_y);
  }
  dpos_t spacings = {.x = mpp_x, .y = mpp_y};
  return spacings;
}

ipos_t get_offset(openslide_t *osr) {
  const char *c_x = openslide_get_property_value(osr, PROPERTY_NAME_BOUNDS_X);
  const char *c_y = openslide_get_property_value(osr, PROPERTY_NAME_BOUNDS_Y);
  int64_t x, y;
  if (!c_x) {
    x = 0;
  } else {
    x = atof(c_x);
  }
  if (!c_y) {
    y = 0;
  } else {
    y = atof(c_y);
  }
  ipos_t offset = {.x = x, .y = y};
  return offset;
}

ipos_t get_bounds(openslide_t *osr) {
  // Set to slide size if not found
  int64_t h, w;
  openslide_get_level0_dimensions(osr, &w, &h);
  const char *c_x =
      openslide_get_property_value(osr, PROPERTY_NAME_BOUNDS_WIDTH);
  const char *c_y =
      openslide_get_property_value(osr, PROPERTY_NAME_BOUNDS_HEIGHT);
  int64_t x, y;
  if (!c_x) {
    x = w;
  } else {
    x = atof(c_x);
  }
  if (!c_y) {
    y = h;
  } else {
    y = atof(c_y);
  }
  ipos_t bounds = {.x = x, .y = y};
  return bounds;
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
