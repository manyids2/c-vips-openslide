#include "ops.h"

// Clip values from 0 to size boundaries.
ipos_t _clip2size(ipos_t a, ipos_t size) {
  // return np.clip(a, (0, 0), size)
  ipos_t clipped = {
      .x = MAX(a.x, 0),
      .y = MAX(a.y, 0),
  };
  clipped.x = MIN(clipped.x, size.x);
  clipped.y = MIN(clipped.y, size.y);
  return clipped;
}

dpos_t _clip2size_d(dpos_t a, dpos_t size) {
  // return np.clip(a, (0, 0), size)
  dpos_t clipped = {
      .x = MAX(a.x, 0),
      .y = MAX(a.y, 0),
  };
  clipped.x = MIN(clipped.x, size.x);
  clipped.y = MIN(clipped.y, size.y);
  return clipped;
}

ipos_t get_scaled_size(ipos_t size, double scaling) {
  // Multiple in doubles and cast to int64_t
  ipos_t ret = {.x = (double)size.x * scaling, .y = (double)size.y * scaling};
  return ret;
}

ipos_t to_int(dpos_t a) {
  ipos_t b = {.x = a.x, .y = a.y};
  return b;
}

dpos_t to_double(ipos_t a) {
  dpos_t b = {.x = a.x, .y = a.y};
  return b;
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

// From openslide python - convert in place
void argb2rgba(uint32_t *buf, int len) {
  int64_t cur;
  for (cur = 0; cur < len; cur++) {
    uint32_t val = buf[cur];
    uint8_t a = val >> 24;
    switch (a) {
    case 0:
      break;
    case 255:
      val = (val << 8) | a;
#ifndef WORDS_BIGENDIAN
      // compiler should optimize this to bswap
      val = (((val & 0x000000ff) << 24) | ((val & 0x0000ff00) << 8) |
             ((val & 0x00ff0000) >> 8) | ((val & 0xff000000) >> 24));
#endif
      buf[cur] = val;
      break;
    default:; // label cannot point to a variable declaration
      uint8_t r = 255 * ((val >> 16) & 0xff) / a;
      uint8_t g = 255 * ((val >> 8) & 0xff) / a;
      uint8_t b = 255 * ((val >> 0) & 0xff) / a;
#ifdef WORDS_BIGENDIAN
      val = r << 24 | g << 16 | b << 8 | a;
#else
      val = a << 24 | b << 16 | g << 8 | r;
#endif
      buf[cur] = val;
      break;
    }
  }
}
