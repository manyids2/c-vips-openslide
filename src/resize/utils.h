#include "except.h"
#include "imaging.h"
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define M_PI 3.1415926535897932384626433832795
#define ROUND_UP(f) ((int)((f) >= 0.0 ? (f) + 0.5F : (f)-0.5F))

struct filter {
  double (*filter)(double x);
  double support;
};

static inline double box_filter(double x) {
  if (x > -0.5 && x <= 0.5) {
    return 1.0;
  }
  return 0.0;
}

static inline double bilinear_filter(double x) {
  if (x < 0.0) {
    x = -x;
  }
  if (x < 1.0) {
    return 1.0 - x;
  }
  return 0.0;
}

static inline double hamming_filter(double x) {
  if (x < 0.0) {
    x = -x;
  }
  if (x == 0.0) {
    return 1.0;
  }
  if (x >= 1.0) {
    return 0.0;
  }
  x = x * M_PI;
  return sin(x) / x * (0.54f + 0.46f * cos(x));
}

static inline double bicubic_filter(double x) {
  /* https://en.wikipedia.org/wiki/Bicubic_interpolation#Bicubic_convolution_algorithm
   */
#define a -0.5
  if (x < 0.0) {
    x = -x;
  }
  if (x < 1.0) {
    return ((a + 2.0) * x - (a + 3.0)) * x * x + 1;
  }
  if (x < 2.0) {
    return (((x - 5) * x + 8) * x - 4) * a;
  }
  return 0.0;
#undef a
}

static inline double sinc_filter(double x) {
  if (x == 0.0) {
    return 1.0;
  }
  x = x * M_PI;
  return sin(x) / x;
}

static inline double lanczos_filter(double x) {
  /* truncated sinc */
  if (-3.0 <= x && x < 3.0) {
    return sinc_filter(x) * sinc_filter(x / 3);
  }
  return 0.0;
}

static struct filter BOX = {box_filter, 0.5};
static struct filter BILINEAR = {bilinear_filter, 1.0};
static struct filter HAMMING = {hamming_filter, 1.0};
static struct filter BICUBIC = {bicubic_filter, 2.0};
static struct filter LANCZOS = {lanczos_filter, 3.0};

/* 8 bits for result. Filter can have negative areas.
   In one cases the sum of the coefficients will be negative,
   in the other it will be more than 1.0. That is why we need
   two extra bits for overflow and int type. */
#define PRECISION_BITS (32 - 8 - 2)

/* Handles values form -640 to 639. */
UINT8 _clip8_lookups[1280] = {
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,
    0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   2,   3,   4,
    5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,  18,  19,
    20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,
    35,  36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
    50,  51,  52,  53,  54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,
    65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,  76,  77,  78,  79,
    80,  81,  82,  83,  84,  85,  86,  87,  88,  89,  90,  91,  92,  93,  94,
    95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107, 108, 109,
    110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124,
    125, 126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139,
    140, 141, 142, 143, 144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154,
    155, 156, 157, 158, 159, 160, 161, 162, 163, 164, 165, 166, 167, 168, 169,
    170, 171, 172, 173, 174, 175, 176, 177, 178, 179, 180, 181, 182, 183, 184,
    185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197, 198, 199,
    200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214,
    215, 216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229,
    230, 231, 232, 233, 234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244,
    245, 246, 247, 248, 249, 250, 251, 252, 253, 254, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255, 255,
    255, 255, 255, 255, 255,
};

UINT8 *clip8_lookups = &_clip8_lookups[640];

static inline UINT8 clip8(int in) {
  return clip8_lookups[in >> PRECISION_BITS];
}

int precompute_coeffs(int inSize, float in0, float in1, int outSize,
                      struct filter *filterp, int **boundsp, double **kkp) {
  double support, scale, filterscale;
  double center, ww, ss;
  int xx, x, ksize, xmin, xmax;
  int *bounds;
  double *kk, *k;

  /* prepare for horizontal stretch */
  filterscale = scale = (double)(in1 - in0) / outSize;
  if (filterscale < 1.0) {
    filterscale = 1.0;
  }

  /* determine support size (length of resampling filter) */
  support = filterp->support * filterscale;

  /* maximum number of coeffs */
  ksize = (int)ceil(support) * 2 + 1;

  // check for overflow
  if (outSize > INT_MAX / (ksize * (int)sizeof(double))) {
    ImagingError_MemoryError();
    return 0;
  }

  /* coefficient buffer */
  /* malloc check ok, overflow checked above */
  kk = malloc(outSize * ksize * sizeof(double));
  if (!kk) {
    ImagingError_MemoryError();
    return 0;
  }

  /* malloc check ok, ksize*sizeof(double) > 2*sizeof(int) */
  bounds = malloc(outSize * 2 * sizeof(int));
  if (!bounds) {
    free(kk);
    ImagingError_MemoryError();
    return 0;
  }

  for (xx = 0; xx < outSize; xx++) {
    center = in0 + (xx + 0.5) * scale;
    ww = 0.0;
    ss = 1.0 / filterscale;
    // Round the value
    xmin = (int)(center - support + 0.5);
    if (xmin < 0) {
      xmin = 0;
    }
    // Round the value
    xmax = (int)(center + support + 0.5);
    if (xmax > inSize) {
      xmax = inSize;
    }
    xmax -= xmin;
    k = &kk[xx * ksize];
    for (x = 0; x < xmax; x++) {
      double w = filterp->filter((x + xmin - center + 0.5) * ss);
      k[x] = w;
      ww += w;
    }
    for (x = 0; x < xmax; x++) {
      if (ww != 0.0) {
        k[x] /= ww;
      }
    }
    // Remaining values should stay empty if they are used despite of xmax.
    for (; x < ksize; x++) {
      k[x] = 0;
    }
    bounds[xx * 2 + 0] = xmin;
    bounds[xx * 2 + 1] = xmax;
  }
  *boundsp = bounds;
  *kkp = kk;
  return ksize;
}

void normalize_coeffs_8bpc(int outSize, int ksize, double *prekk) {
  int x;
  INT32 *kk;

  // use the same buffer for normalized coefficients
  kk = (INT32 *)prekk;

  for (x = 0; x < outSize * ksize; x++) {
    if (prekk[x] < 0) {
      kk[x] = (int)(-0.5 + prekk[x] * (1 << PRECISION_BITS));
    } else {
      kk[x] = (int)(0.5 + prekk[x] * (1 << PRECISION_BITS));
    }
  }
}
