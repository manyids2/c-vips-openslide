#include "ops.h"

// Clip values from 0 to size boundaries.
dpos_t _clip2size(dpos_t a, ipos_t size) {
  // return np.clip(a, (0, 0), size)
  dpos_t clipped = {
      .x = MAX(a.x, 0),
      .y = MAX(a.y, 0),
  };
  clipped.x = MIN(clipped.x, size.x);
  clipped.y = MIN(clipped.x, size.y);
  return clipped;
}

ipos_t get_scaled_size(ipos_t size, double scaling) {
  // Multiple in doubles and cast to int64_t
  ipos_t ret = {.x = (double)size.x * scaling, .y = (double)size.y * scaling};
  return ret;
}

// Divide vector by scalar
dpos_t _add(dpos_t a, double b) {
  dpos_t ret = {.x = a.x + b, .y = a.y + b};
  return ret;
}

dpos_t _sub(dpos_t a, double b) {
  dpos_t ret = {.x = a.x - b, .y = a.y - b};
  return ret;
}

dpos_t _mul(dpos_t a, double b) {
  dpos_t ret = {.x = (double)a.x * b, .y = (double)a.y * b};
  return ret;
}

dpos_t _div(dpos_t a, double b) {
  dpos_t ret = {.x = (double)a.x / b, .y = (double)a.y / b};
  return ret;
}

dpos_t _floor(dpos_t a) {
  dpos_t ret = {.x = floor(a.x), .y = floor(a.y)};
  return ret;
}

dpos_t _ceil(dpos_t a) {
  dpos_t ret = {.x = ceil(a.x), .y = ceil(a.y)};
  return ret;
}

dpos_t _addv(dpos_t a, dpos_t b) {
  dpos_t ret = {.x = a.x + b.x, .y = a.y + b.y};
  return ret;
}

dpos_t _subv(dpos_t a, dpos_t b) {
  dpos_t ret = {.x = a.x - b.x, .y = a.y - b.y};
  return ret;
}
