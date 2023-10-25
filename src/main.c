#include "slide.h"
#include "types.h"

int main(void) {
  char *path = "/data/slides/Aperio/CMU-1.svs\0";
  printf("path: %s\r\n", path);

  // Load all props
  oslide_t oslide = oslide_open(path);
  oslide_print(&oslide);

  // Input:
  //   location: 6912,5376
  //   scaling : 0.2495
  //   size    : 256,256

  // Output:
  //   location: 27684,21528
  //   level   : 1
  //   size    : 267,267
  //     fractional : 4.851555996871866,4.773402058879583
  //     native_size: 256.50523238315725,256.50523238315725

  // Read region args
  ipos_t location = {.x = 6912, .y = 5376};
  double scaling = 0.2495;
  ipos_t size = {.x = 256, .y = 256};
  printf("location: %7ld, %7ld\n", location.x, location.y);
  printf("scaling : %f\n", scaling);
  printf("size    : %7ld, %7ld\n", size.x, size.y);

  request_t request = read_region_request(location, scaling, size, oslide.osr,
                                          oslide.level_props);
  print_request(request);

  // Go further and get the region
  image_t region = {
      .width = request.size.x,
      .height = request.size.y,
      .bands = 4,
      .data = malloc(request.size.x * request.size.y * sizeof(uint32_t))};
  read_region(&region, oslide.osr, request);

  // Close and free
  oslide_close(&oslide);

  return 0;
}
