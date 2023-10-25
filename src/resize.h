#include "slide.h"
#include <stdlib.h>
#include <vips/vips.h>

// Wrappers to vips
int image_resize(image_t *out, image_t *in, ipos_t size, VipsKernel resampling);
int image_rescale(image_t *out, image_t *in, double scaling,
                  VipsKernel resampling);
