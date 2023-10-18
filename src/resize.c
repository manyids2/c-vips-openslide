#include "resize.h"

int resize_image(image_t *out, image_t *in, ipos_t size,
                 VipsKernel resampling) {
  VipsImage *img = vips_image_new_from_memory(
      in->data, (in->width * in->height * in->bands), in->width, in->height,
      in->bands, VIPS_FORMAT_CHAR);

  double scale_x = (double)size.x / (double)in->width;
  double scale_y = (double)size.y / (double)in->height;

  VipsImage *rsz;
  int err = vips_resize(img, &rsz, scale_x, scale_y, resampling, NULL);

  // Assign to out, now out owns the memory
  out->width = size.x;
  out->height = size.y;
  out->bands = in->bands;
  out->data = rsz->data;

  return err;
}

int rescale_image(image_t *out, image_t *in, double scaling,
                  VipsKernel resampling) {
  VipsImage *img = vips_image_new_from_memory(
      in->data, (in->width * in->height * in->bands), in->width, in->height,
      in->bands, VIPS_FORMAT_CHAR);

  int size_x = (double)in->width * scaling;
  int size_y = (double)in->height * scaling;

  VipsImage *rsz;
  int err = vips_resize(img, &rsz, scaling, NULL, resampling, NULL);

  // Assign to out, now out owns the memory
  out->width = size_x;
  out->height = size_y;
  out->bands = in->bands;
  out->data = rsz->data;

  return err;
}
