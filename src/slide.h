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
oslide_t oslide_open(char *path);
void oslide_close(oslide_t *oslide);
void oslide_print(oslide_t *oslide);

// --- Extensions to openslide_t ---

// Images
int osr_length_associated_images(openslide_t *osr);
int osr_thumbnail(openslide_t *osr, image_t *thumbnail, AssociatedImage name);

// mpp stuff
double osr_mpp(openslide_t *osr);
double osr_magnification(openslide_t *osr);
dpos_t osr_spacings(openslide_t *osr); // mpp_x, mpp_y

// size stuff
ipos_t osr_size(openslide_t *osr);
ipos_t osr_offset(openslide_t *osr);
ipos_t osr_bounds(openslide_t *osr);
double osr_scaling(openslide_t *osr, double mpp); // TODO:

// level stuff - putting in output for mem management
void osr_level_downsamples(openslide_t *osr, int level_count,
                           double *level_downsamples);
void osr_level_dimensions(openslide_t *osr, int level_count,
                          ipos_t *level_dimensions);

// NOTE: MVP - Get only request params, along with adjustments for sampling
int is_valid_region(ipos_t location, double scaling, ipos_t size,
                    level_props_t level_props);
request_t read_region_request(ipos_t location, double scaling, ipos_t size,
                              openslide_t *osr, level_props_t level_props);
void print_request(request_t request);

// NOTE: Actual sauce, read and resize
int read_region(image_t *region, openslide_t *osr, request_t request);

// Helpers to dump to csv
void print_lss_header(void);
void print_lss_row(ipos_t location, double scaling, ipos_t size);
void print_request_header(void);
void print_request_row(request_t);
