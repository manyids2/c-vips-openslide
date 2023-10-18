#include "ops.h"
#include <math.h>
#include <openslide/openslide.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Utils
char *str_at(char **p, int i);

// Images
int length_associated_images(openslide_t *osr);
int get_thumbnail(image_t *thumbnail, AssociatedImage name);

// mpp stuff
double get_mpp(openslide_t *osr);
double get_magnification(openslide_t *osr);
dpos_t get_spacings(openslide_t *osr); // mpp_x, mpp_y

// size stuff
ipos_t get_size(openslide_t *osr);
ipos_t get_offset(openslide_t *osr);
ipos_t get_bounds(openslide_t *osr);

// Scaling helpers
double get_scaling(double mpp);
ipos_t get_scaled_size(ipos_t size, double scaling);

// Open, close

// MVP - takes output value in args for better memory management
int read_region_request(request_t *request, ipos_t location, double scaling,
                        ipos_t size, openslide_t *osr,
                        level_props_t *level_props);

// int read_region(image_t *region, openslide_t *osr, request_t request,
//                 Resampling resampling);
