#include "slide.h"
#include <stdlib.h>
#include <vips/vips.h>

// Wrappers to vips
int resize_image(image_t *out, image_t *in, ipos_t size, VipsKernel resampling);
int rescale_image(image_t *out, image_t *in, double scaling,
                  VipsKernel resampling);
