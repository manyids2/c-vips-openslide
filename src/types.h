#pragma once

#include <math.h>
#include <openslide/openslide.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Utils
#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

char *str_at(char **p, int i);

typedef struct ipos_t {
  int64_t x, y;
} ipos_t;

typedef struct dpos_t {
  double x, y;
} dpos_t;

typedef struct dbox_t {
  double x1, y1, x2, y2;
} dbox_t;

// Images
typedef struct image_t {
  int width, height, bands;
  uint8_t *data; // From openslide -> ARGB, 4 * uint8
} image_t;

typedef enum AssociatedImage {
  Thumbnail = 1,
  Label,
  Macro,
} AssociatedImage;

inline static const char *stringFromAssociatedImage(enum AssociatedImage f) {
  static const char *strings[] = {"thumbnail\0", "label\0", "macro\0"};
  return strings[f];
}

// Slide dimensions properties
typedef struct slide_props_t {
  double mpp;
  double magnification;
  dpos_t spacings;
  ipos_t size, offset, bounds;
} slide_props_t;

// Level dims, downsamples, etc.
typedef struct level_props_t {
  ipos_t slide_size;
  int level_count;
  double *level_downsamples;
  ipos_t *level_dimensions;
} level_props_t;

// Params to request read_region from openslide
typedef struct native_t {
  dpos_t fractional_coordinates;
  dpos_t native_size;
} native_t;

typedef struct request_t {
  ipos_t location;
  int level;
  ipos_t size;
  native_t native;
} request_t;