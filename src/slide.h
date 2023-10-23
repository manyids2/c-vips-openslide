#include "ops.h"
#include <math.h>
#include <openslide/openslide.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Main struct to hold everything
typedef struct oslide_t {
  char *path;
  openslide_t *osr;
  slide_props_t slide_props;
  level_props_t level_props;
} oslide_t;

// Open, close
oslide_t open_oslide(char *path);
void close_oslide(oslide_t *oslide);

// Utils
char *str_at(char **p, int i);

// Images
int length_associated_images(openslide_t *osr);
int get_thumbnail(openslide_t *osr, image_t *thumbnail, AssociatedImage name);

// mpp stuff
double get_mpp(openslide_t *osr);
double get_magnification(openslide_t *osr);
dpos_t get_spacings(openslide_t *osr); // mpp_x, mpp_y

// size stuff
ipos_t get_size(openslide_t *osr);
ipos_t get_offset(openslide_t *osr);
ipos_t get_bounds(openslide_t *osr);

// level stuff - putting in output for mem management
void get_level_downsamples(openslide_t *osr, int level_count,
                           double *level_downsamples);
void get_level_dimensions(openslide_t *osr, int level_count,
                          ipos_t *level_dimensions);

void print_slide(oslide_t *oslide);

// Scaling helpers
double get_scaling(double mpp);
ipos_t get_scaled_size(ipos_t size, double scaling);

// MVP - takes output value in args for better memory management
int read_region_request(request_t *request, ipos_t location, double scaling,
                        ipos_t size, openslide_t *osr,
                        level_props_t *level_props);

// int read_region(image_t *region, openslide_t *osr, request_t request,
//                 Resampling resampling);
