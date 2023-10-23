#include "slide.h"
#include "types.h"
#include <stdlib.h>

#define BUFFERLENGTH 255

#define CASE_FLOAT(i, var)                                              \
  case i:                                                                      \
    var = atof(token);                                                         \
    break;

int read_csv(char *path, oslide_t *oslide) {
  FILE *filePointer;
  char buffer[BUFFERLENGTH];

  filePointer = fopen(path, "r");
  int counter = 0;
  while (fgets(buffer, BUFFERLENGTH, filePointer)) {
    // printf("%s", buffer);
    char *token = strtok(buffer, ",");
    int i = 0;

    // Need to initialize...
    ipos_t location = {.x=0, .y=0};
    double scaling = 1.0;
    ipos_t size = {.x=0, .y=0};
    request_t request_A;
    while (token) {
      switch (i) {
        CASE_FLOAT(0, location.x)
        CASE_FLOAT(1, location.y)
        CASE_FLOAT(2, scaling)
        CASE_FLOAT(3, size.x)
        CASE_FLOAT(4, size.y)
        CASE_FLOAT(5, request_A.location.x)
        CASE_FLOAT(6, request_A.location.y)
        CASE_FLOAT(7, request_A.level)
        CASE_FLOAT(8, request_A.size.x)
        CASE_FLOAT(9, request_A.size.y)
        CASE_FLOAT(10, request_A.native.fractional_coordinates.x)
        CASE_FLOAT(11, request_A.native.fractional_coordinates.y)
        CASE_FLOAT(12, request_A.native.native_size.x)
        CASE_FLOAT(13, request_A.native.native_size.y)
      }
      i++;
      token = strtok(NULL, ",");
    }

    print_lss_row(location, scaling, size);

    request_t request_B;
    if (is_valid_region(location, scaling, size, oslide->level_props) &&
        (counter > 0)) {
      request_B = read_region_request(location, scaling, size, oslide->osr,
                                      oslide->level_props);
      // print_lss_header();
      // print_request_header();
      print_lss_row(location, scaling, size);
      printf("\n");
      print_request_row(request_A);
      print_request_row(request_B);
      printf("\n");

      printf("Diff:\n"
             "  location: %7ld, %7ld\n"
             "  level   : %7d\n"
             "  size    : %7ld, %7ld\n"
             " native:\n"
             "    frac  : %f, %f\n"
             "    size  : %f, %f\n",
             (request_B.location.x - request_A.location.x),
             (request_B.location.y - request_A.location.y),
             (request_B.level - request_A.level),
             (request_B.size.x - request_A.size.x),
             (request_B.size.y - request_A.size.y),
             (request_B.native.fractional_coordinates.x -
              request_A.native.fractional_coordinates.x),
             (request_B.native.fractional_coordinates.y -
              request_A.native.fractional_coordinates.y),
             (request_B.native.native_size.x - request_A.native.native_size.x),
             (request_B.native.native_size.y - request_A.native.native_size.y));
    }
    counter = 1;
  }
  fclose(filePointer);
  return 1;
}

int main(void) {
  char *path = "/data/slides/Aperio/CMU-1.svs\0";
  printf("path: %s\r\n", path);

  // Load all props
  oslide_t oslide = open_oslide(path);
  print_slide(&oslide);

  // Read region and get request
  read_csv("requests.csv", &oslide);

  // Close and free
  close_oslide(&oslide);

  return 0;
}

