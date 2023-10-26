#include "slide.h"
#include "types.h"

int main(void) {
  char *path = "/data/slides/Aperio/CMU-1.svs\0";
  printf("path: %s\r\n", path);

  // Load all props
  oslide_t oslide = oslide_open(path);
  oslide_print(&oslide);

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

  // BUG: FAILING
  // Go further and get the region
  // image_t region = {
  //     .width = size.x,
  //     .height = size.y,
  //     .bands = 4,
  //     .data = malloc(size.x * size.y * sizeof(uint32_t))};
  // read_region(&region, oslide.osr, request);

  // Close and free
  oslide_close(&oslide);

  return 0;
}
