#include "utils.h"

//-------------------------------------------------------------------------
//                    -- Memory stuff --
//-------------------------------------------------------------------------

Imaging ImagingNewPrologueSubtype(const char *mode, int xsize, int ysize,
                                  int size) {
  Imaging im;

  /* linesize overflow check, roughly the current largest space req'd */
  if (xsize > (INT_MAX / 4) - 1) {
    return (Imaging)ImagingError_MemoryError();
  }

  im = (Imaging)calloc(1, size);
  if (!im) {
    return (Imaging)ImagingError_MemoryError();
  }

  /* Setup image descriptor */
  im->xsize = xsize;
  im->ysize = ysize;

  im->type = IMAGING_TYPE_UINT8;

  if (strcmp(mode, "1") == 0) {
    /* 1-bit images */
    im->bands = im->pixelsize = 1;
    im->linesize = xsize;

    // NOTE: removed "P" and "PA" modes ( palette )
  } else if (strcmp(mode, "L") == 0) {
    /* 8-bit grayscale (luminance) images */
    im->bands = im->pixelsize = 1;
    im->linesize = xsize;

  } else if (strcmp(mode, "LA") == 0) {
    /* 8-bit grayscale (luminance) with alpha */
    im->bands = 2;
    im->pixelsize = 4; /* store in image32 memory */
    im->linesize = xsize * 4;

  } else if (strcmp(mode, "La") == 0) {
    /* 8-bit grayscale (luminance) with premultiplied alpha */
    im->bands = 2;
    im->pixelsize = 4; /* store in image32 memory */
    im->linesize = xsize * 4;

  } else if (strcmp(mode, "F") == 0) {
    /* 32-bit floating point images */
    im->bands = 1;
    im->pixelsize = 4;
    im->linesize = xsize * 4;
    im->type = IMAGING_TYPE_FLOAT32;

  } else if (strcmp(mode, "I") == 0) {
    /* 32-bit integer images */
    im->bands = 1;
    im->pixelsize = 4;
    im->linesize = xsize * 4;
    im->type = IMAGING_TYPE_INT32;

  } else if (strcmp(mode, "I;16") == 0 || strcmp(mode, "I;16L") == 0 ||
             strcmp(mode, "I;16B") == 0 || strcmp(mode, "I;16N") == 0) {
    /* EXPERIMENTAL */
    /* 16-bit raw integer images */
    im->bands = 1;
    im->pixelsize = 2;
    im->linesize = xsize * 2;
    im->type = IMAGING_TYPE_SPECIAL;

  } else if (strcmp(mode, "RGB") == 0) {
    /* 24-bit true colour images */
    im->bands = 3;
    im->pixelsize = 4;
    im->linesize = xsize * 4;

  } else if (strcmp(mode, "BGR;15") == 0) {
    /* EXPERIMENTAL */
    /* 15-bit reversed true colour */
    im->bands = 3;
    im->pixelsize = 2;
    im->linesize = (xsize * 2 + 3) & -4;
    im->type = IMAGING_TYPE_SPECIAL;

  } else if (strcmp(mode, "BGR;16") == 0) {
    /* EXPERIMENTAL */
    /* 16-bit reversed true colour */
    im->bands = 3;
    im->pixelsize = 2;
    im->linesize = (xsize * 2 + 3) & -4;
    im->type = IMAGING_TYPE_SPECIAL;

  } else if (strcmp(mode, "BGR;24") == 0) {
    /* EXPERIMENTAL */
    /* 24-bit reversed true colour */
    im->bands = 3;
    im->pixelsize = 3;
    im->linesize = (xsize * 3 + 3) & -4;
    im->type = IMAGING_TYPE_SPECIAL;

  } else if (strcmp(mode, "RGBX") == 0) {
    /* 32-bit true colour images with padding */
    im->bands = im->pixelsize = 4;
    im->linesize = xsize * 4;

  } else if (strcmp(mode, "RGBA") == 0) {
    /* 32-bit true colour images with alpha */
    im->bands = im->pixelsize = 4;
    im->linesize = xsize * 4;

  } else if (strcmp(mode, "RGBa") == 0) {
    /* 32-bit true colour images with premultiplied alpha */
    im->bands = im->pixelsize = 4;
    im->linesize = xsize * 4;

  } else if (strcmp(mode, "CMYK") == 0) {
    /* 32-bit colour separation */
    im->bands = im->pixelsize = 4;
    im->linesize = xsize * 4;

  } else if (strcmp(mode, "YCbCr") == 0) {
    /* 24-bit video format */
    im->bands = 3;
    im->pixelsize = 4;
    im->linesize = xsize * 4;

  } else if (strcmp(mode, "LAB") == 0) {
    /* 24-bit color, luminance, + 2 color channels */
    /* L is uint8, a,b are int8 */
    im->bands = 3;
    im->pixelsize = 4;
    im->linesize = xsize * 4;

  } else if (strcmp(mode, "HSV") == 0) {
    /* 24-bit color, luminance, + 2 color channels */
    /* L is uint8, a,b are int8 */
    im->bands = 3;
    im->pixelsize = 4;
    im->linesize = xsize * 4;

  } else {
    free(im);
    return (Imaging)ImagingError_ValueError("unrecognized image mode");
  }

  /* Setup image descriptor */
  strcpy(im->mode, mode);

  /* Pointer array (allocate at least one line, to avoid MemoryError
     exceptions on platforms where calloc(0, x) returns NULL) */
  im->image = (char **)calloc((ysize > 0) ? ysize : 1, sizeof(void *));

  if (!im->image) {
    free(im);
    return (Imaging)ImagingError_MemoryError();
  }

  /* Initialize alias pointers to pixel data. */
  switch (im->pixelsize) {
  case 1:
  case 2:
  case 3:
    im->image8 = (UINT8 **)im->image;
    break;
  case 4:
    im->image32 = (INT32 **)im->image;
    break;
  }

  ImagingDefaultArena.stats_new_count += 1;

  return im;
}

Imaging ImagingNewPrologue(const char *mode, int xsize, int ysize) {
  return ImagingNewPrologueSubtype(mode, xsize, ysize,
                                   sizeof(struct ImagingMemoryInstance));
}

void ImagingDelete(Imaging im) {
  if (!im) {
    return;
  }

  // NOTE: removed free for im->palette

  if (im->destroy) {
    im->destroy(im);
  }

  if (im->image) {
    free(im->image);
  }

  free(im);
}

#define IMAGING_PAGE_SIZE (4096)

struct ImagingMemoryArena ImagingDefaultArena = {
    1,                // alignment
    16 * 1024 * 1024, // block_size
    0,                // blocks_max
    0,                // blocks_cached
    NULL,             // blocks_pool
    0,
    0,
    0,
    0,
    0 // Stats
};

int ImagingMemorySetBlocksMax(ImagingMemoryArena arena, int blocks_max) {
  void *p;
  /* Free already cached blocks */
  ImagingMemoryClearCache(arena, blocks_max);

  if (blocks_max == 0 && arena->blocks_pool != NULL) {
    free(arena->blocks_pool);
    arena->blocks_pool = NULL;
  } else if (arena->blocks_pool != NULL) {
    p = realloc(arena->blocks_pool, sizeof(*arena->blocks_pool) * blocks_max);
    if (!p) {
      // Leave previous blocks_max value
      return 0;
    }
    arena->blocks_pool = p;
  } else {
    arena->blocks_pool = calloc(sizeof(*arena->blocks_pool), blocks_max);
    if (!arena->blocks_pool) {
      return 0;
    }
  }
  arena->blocks_max = blocks_max;

  return 1;
}

void ImagingMemoryClearCache(ImagingMemoryArena arena, int new_size) {
  while (arena->blocks_cached > new_size) {
    arena->blocks_cached -= 1;
    free(arena->blocks_pool[arena->blocks_cached].ptr);
    arena->stats_freed_blocks += 1;
  }
}

ImagingMemoryBlock memory_get_block(ImagingMemoryArena arena,
                                    int requested_size, int dirty) {
  ImagingMemoryBlock block = {NULL, 0};

  if (arena->blocks_cached > 0) {
    // Get block from cache
    arena->blocks_cached -= 1;
    block = arena->blocks_pool[arena->blocks_cached];
    // Reallocate if needed
    if (block.size != requested_size) {
      block.ptr = realloc(block.ptr, requested_size);
    }
    if (!block.ptr) {
      // Can't allocate, free previous pointer (it is still valid)
      free(arena->blocks_pool[arena->blocks_cached].ptr);
      arena->stats_freed_blocks += 1;
      return block;
    }
    if (!dirty) {
      memset(block.ptr, 0, requested_size);
    }
    arena->stats_reused_blocks += 1;
    if (block.ptr != arena->blocks_pool[arena->blocks_cached].ptr) {
      arena->stats_reallocated_blocks += 1;
    }
  } else {
    if (dirty) {
      block.ptr = malloc(requested_size);
    } else {
      block.ptr = calloc(1, requested_size);
    }
    arena->stats_allocated_blocks += 1;
  }
  block.size = requested_size;
  return block;
}

void memory_return_block(ImagingMemoryArena arena, ImagingMemoryBlock block) {
  if (arena->blocks_cached < arena->blocks_max) {
    // Reduce block size
    if (block.size > arena->block_size) {
      block.size = arena->block_size;
      block.ptr = realloc(block.ptr, arena->block_size);
    }
    arena->blocks_pool[arena->blocks_cached] = block;
    arena->blocks_cached += 1;
  } else {
    free(block.ptr);
    arena->stats_freed_blocks += 1;
  }
}

static void ImagingDestroyArray(Imaging im) {
  int y = 0;

  if (im->blocks) {
    while (im->blocks[y].ptr) {
      memory_return_block(&ImagingDefaultArena, im->blocks[y]);
      y += 1;
    }
    free(im->blocks);
  }
}

Imaging ImagingAllocateArray(Imaging im, int dirty, int block_size) {
  int y, line_in_block, current_block;
  ImagingMemoryArena arena = &ImagingDefaultArena;
  ImagingMemoryBlock block = {NULL, 0};
  int aligned_linesize, lines_per_block, blocks_count;
  char *aligned_ptr = NULL;

  /* 0-width or 0-height image. No need to do anything */
  if (!im->linesize || !im->ysize) {
    return im;
  }

  aligned_linesize = (im->linesize + arena->alignment - 1) & -arena->alignment;
  lines_per_block = (block_size - (arena->alignment - 1)) / aligned_linesize;
  if (lines_per_block == 0) {
    lines_per_block = 1;
  }
  blocks_count = (im->ysize + lines_per_block - 1) / lines_per_block;
  // printf("NEW size: %dx%d, ls: %d, lpb: %d, blocks: %d\n",
  //        im->xsize, im->ysize, aligned_linesize, lines_per_block,
  //        blocks_count);

  /* One extra pointer is always NULL */
  im->blocks = calloc(sizeof(*im->blocks), blocks_count + 1);
  if (!im->blocks) {
    return (Imaging)ImagingError_MemoryError();
  }

  /* Allocate image as an array of lines */
  line_in_block = 0;
  current_block = 0;
  for (y = 0; y < im->ysize; y++) {
    if (line_in_block == 0) {
      int required;
      int lines_remaining = lines_per_block;
      if (lines_remaining > im->ysize - y) {
        lines_remaining = im->ysize - y;
      }
      required = lines_remaining * aligned_linesize + arena->alignment - 1;
      block = memory_get_block(arena, required, dirty);
      if (!block.ptr) {
        ImagingDestroyArray(im);
        return (Imaging)ImagingError_MemoryError();
      }
      im->blocks[current_block] = block;
      /* Bulletproof code from libc _int_memalign */
      aligned_ptr = (char *)(((size_t)(block.ptr + arena->alignment - 1)) &
                             -((__ssize_t)arena->alignment));
    }

    im->image[y] = aligned_ptr + aligned_linesize * line_in_block;

    line_in_block += 1;
    if (line_in_block >= lines_per_block) {
      /* Reset counter and start new block */
      line_in_block = 0;
      current_block += 1;
    }
  }

  im->destroy = ImagingDestroyArray;

  return im;
}

//-------------------------------------------------------------------------
//                    -- Actual resize stuff --
//-------------------------------------------------------------------------

/* standard filters */
#define IMAGING_TRANSFORM_NEAREST 0
#define IMAGING_TRANSFORM_BOX 4
#define IMAGING_TRANSFORM_BILINEAR 2
#define IMAGING_TRANSFORM_HAMMING 5
#define IMAGING_TRANSFORM_BICUBIC 3
#define IMAGING_TRANSFORM_LANCZOS 1

void ImagingResampleHorizontal_8bpc(Imaging imOut, Imaging imIn, int offset,
                                    int ksize, int *bounds, double *prekk) {
  int ss0, ss1, ss2, ss3;
  int xx, yy, x, xmin, xmax;
  INT32 *k, *kk;

  // use the same buffer for normalized coefficients
  kk = (INT32 *)prekk;
  normalize_coeffs_8bpc(imOut->xsize, ksize, prekk);

  if (imIn->image8) {
    for (yy = 0; yy < imOut->ysize; yy++) {
      for (xx = 0; xx < imOut->xsize; xx++) {
        xmin = bounds[xx * 2 + 0];
        xmax = bounds[xx * 2 + 1];
        k = &kk[xx * ksize];
        ss0 = 1 << (PRECISION_BITS - 1);
        for (x = 0; x < xmax; x++) {
          ss0 += ((UINT8)imIn->image8[yy + offset][x + xmin]) * k[x];
        }
        imOut->image8[yy][xx] = clip8(ss0);
      }
    }
  } else if (imIn->type == IMAGING_TYPE_UINT8) {
    if (imIn->bands == 2) {
      for (yy = 0; yy < imOut->ysize; yy++) {
        for (xx = 0; xx < imOut->xsize; xx++) {
          UINT32 v;
          xmin = bounds[xx * 2 + 0];
          xmax = bounds[xx * 2 + 1];
          k = &kk[xx * ksize];
          ss0 = ss3 = 1 << (PRECISION_BITS - 1);
          for (x = 0; x < xmax; x++) {
            ss0 += ((UINT8)imIn->image[yy + offset][(x + xmin) * 4 + 0]) * k[x];
            ss3 += ((UINT8)imIn->image[yy + offset][(x + xmin) * 4 + 3]) * k[x];
          }
          v = MAKE_UINT32(clip8(ss0), 0, 0, clip8(ss3));
          memcpy(imOut->image[yy] + xx * sizeof(v), &v, sizeof(v));
        }
      }
    } else if (imIn->bands == 3) {
      for (yy = 0; yy < imOut->ysize; yy++) {
        for (xx = 0; xx < imOut->xsize; xx++) {
          UINT32 v;
          xmin = bounds[xx * 2 + 0];
          xmax = bounds[xx * 2 + 1];
          k = &kk[xx * ksize];
          ss0 = ss1 = ss2 = 1 << (PRECISION_BITS - 1);
          for (x = 0; x < xmax; x++) {
            ss0 += ((UINT8)imIn->image[yy + offset][(x + xmin) * 4 + 0]) * k[x];
            ss1 += ((UINT8)imIn->image[yy + offset][(x + xmin) * 4 + 1]) * k[x];
            ss2 += ((UINT8)imIn->image[yy + offset][(x + xmin) * 4 + 2]) * k[x];
          }
          v = MAKE_UINT32(clip8(ss0), clip8(ss1), clip8(ss2), 0);
          memcpy(imOut->image[yy] + xx * sizeof(v), &v, sizeof(v));
        }
      }
    } else {
      for (yy = 0; yy < imOut->ysize; yy++) {
        for (xx = 0; xx < imOut->xsize; xx++) {
          UINT32 v;
          xmin = bounds[xx * 2 + 0];
          xmax = bounds[xx * 2 + 1];
          k = &kk[xx * ksize];
          ss0 = ss1 = ss2 = ss3 = 1 << (PRECISION_BITS - 1);
          for (x = 0; x < xmax; x++) {
            ss0 += ((UINT8)imIn->image[yy + offset][(x + xmin) * 4 + 0]) * k[x];
            ss1 += ((UINT8)imIn->image[yy + offset][(x + xmin) * 4 + 1]) * k[x];
            ss2 += ((UINT8)imIn->image[yy + offset][(x + xmin) * 4 + 2]) * k[x];
            ss3 += ((UINT8)imIn->image[yy + offset][(x + xmin) * 4 + 3]) * k[x];
          }
          v = MAKE_UINT32(clip8(ss0), clip8(ss1), clip8(ss2), clip8(ss3));
          memcpy(imOut->image[yy] + xx * sizeof(v), &v, sizeof(v));
        }
      }
    }
  }
}

void ImagingResampleVertical_8bpc(Imaging imOut, Imaging imIn, int offset,
                                  int ksize, int *bounds, double *prekk) {
  int ss0, ss1, ss2, ss3;
  int xx, yy, y, ymin, ymax;
  INT32 *k, *kk;

  // use the same buffer for normalized coefficients
  kk = (INT32 *)prekk;
  normalize_coeffs_8bpc(imOut->ysize, ksize, prekk);

  if (imIn->image8) {
    for (yy = 0; yy < imOut->ysize; yy++) {
      k = &kk[yy * ksize];
      ymin = bounds[yy * 2 + 0];
      ymax = bounds[yy * 2 + 1];
      for (xx = 0; xx < imOut->xsize; xx++) {
        ss0 = 1 << (PRECISION_BITS - 1);
        for (y = 0; y < ymax; y++) {
          ss0 += ((UINT8)imIn->image8[y + ymin][xx]) * k[y];
        }
        imOut->image8[yy][xx] = clip8(ss0);
      }
    }
  } else if (imIn->type == IMAGING_TYPE_UINT8) {
    if (imIn->bands == 2) {
      for (yy = 0; yy < imOut->ysize; yy++) {
        k = &kk[yy * ksize];
        ymin = bounds[yy * 2 + 0];
        ymax = bounds[yy * 2 + 1];
        for (xx = 0; xx < imOut->xsize; xx++) {
          UINT32 v;
          ss0 = ss3 = 1 << (PRECISION_BITS - 1);
          for (y = 0; y < ymax; y++) {
            ss0 += ((UINT8)imIn->image[y + ymin][xx * 4 + 0]) * k[y];
            ss3 += ((UINT8)imIn->image[y + ymin][xx * 4 + 3]) * k[y];
          }
          v = MAKE_UINT32(clip8(ss0), 0, 0, clip8(ss3));
          memcpy(imOut->image[yy] + xx * sizeof(v), &v, sizeof(v));
        }
      }
    } else if (imIn->bands == 3) {
      for (yy = 0; yy < imOut->ysize; yy++) {
        k = &kk[yy * ksize];
        ymin = bounds[yy * 2 + 0];
        ymax = bounds[yy * 2 + 1];
        for (xx = 0; xx < imOut->xsize; xx++) {
          UINT32 v;
          ss0 = ss1 = ss2 = 1 << (PRECISION_BITS - 1);
          for (y = 0; y < ymax; y++) {
            ss0 += ((UINT8)imIn->image[y + ymin][xx * 4 + 0]) * k[y];
            ss1 += ((UINT8)imIn->image[y + ymin][xx * 4 + 1]) * k[y];
            ss2 += ((UINT8)imIn->image[y + ymin][xx * 4 + 2]) * k[y];
          }
          v = MAKE_UINT32(clip8(ss0), clip8(ss1), clip8(ss2), 0);
          memcpy(imOut->image[yy] + xx * sizeof(v), &v, sizeof(v));
        }
      }
    } else {
      for (yy = 0; yy < imOut->ysize; yy++) {
        k = &kk[yy * ksize];
        ymin = bounds[yy * 2 + 0];
        ymax = bounds[yy * 2 + 1];
        for (xx = 0; xx < imOut->xsize; xx++) {
          UINT32 v;
          ss0 = ss1 = ss2 = ss3 = 1 << (PRECISION_BITS - 1);
          for (y = 0; y < ymax; y++) {
            ss0 += ((UINT8)imIn->image[y + ymin][xx * 4 + 0]) * k[y];
            ss1 += ((UINT8)imIn->image[y + ymin][xx * 4 + 1]) * k[y];
            ss2 += ((UINT8)imIn->image[y + ymin][xx * 4 + 2]) * k[y];
            ss3 += ((UINT8)imIn->image[y + ymin][xx * 4 + 3]) * k[y];
          }
          v = MAKE_UINT32(clip8(ss0), clip8(ss1), clip8(ss2), clip8(ss3));
          memcpy(imOut->image[yy] + xx * sizeof(v), &v, sizeof(v));
        }
      }
    }
  }
}

void ImagingResampleHorizontal_32bpc(Imaging imOut, Imaging imIn, int offset,
                                     int ksize, int *bounds, double *kk) {
  double ss;
  int xx, yy, x, xmin, xmax;
  double *k;

  switch (imIn->type) {
  case IMAGING_TYPE_INT32:
    for (yy = 0; yy < imOut->ysize; yy++) {
      for (xx = 0; xx < imOut->xsize; xx++) {
        xmin = bounds[xx * 2 + 0];
        xmax = bounds[xx * 2 + 1];
        k = &kk[xx * ksize];
        ss = 0.0;
        for (x = 0; x < xmax; x++) {
          ss += IMAGING_PIXEL_I(imIn, x + xmin, yy + offset) * k[x];
        }
        IMAGING_PIXEL_I(imOut, xx, yy) = ROUND_UP(ss);
      }
    }
    break;

  case IMAGING_TYPE_FLOAT32:
    for (yy = 0; yy < imOut->ysize; yy++) {
      for (xx = 0; xx < imOut->xsize; xx++) {
        xmin = bounds[xx * 2 + 0];
        xmax = bounds[xx * 2 + 1];
        k = &kk[xx * ksize];
        ss = 0.0;
        for (x = 0; x < xmax; x++) {
          ss += IMAGING_PIXEL_F(imIn, x + xmin, yy + offset) * k[x];
        }
        IMAGING_PIXEL_F(imOut, xx, yy) = ss;
      }
    }
    break;
  }
}

void ImagingResampleVertical_32bpc(Imaging imOut, Imaging imIn, int offset,
                                   int ksize, int *bounds, double *kk) {
  double ss;
  int xx, yy, y, ymin, ymax;
  double *k;

  switch (imIn->type) {
  case IMAGING_TYPE_INT32:
    for (yy = 0; yy < imOut->ysize; yy++) {
      ymin = bounds[yy * 2 + 0];
      ymax = bounds[yy * 2 + 1];
      k = &kk[yy * ksize];
      for (xx = 0; xx < imOut->xsize; xx++) {
        ss = 0.0;
        for (y = 0; y < ymax; y++) {
          ss += IMAGING_PIXEL_I(imIn, xx, y + ymin) * k[y];
        }
        IMAGING_PIXEL_I(imOut, xx, yy) = ROUND_UP(ss);
      }
    }
    break;

  case IMAGING_TYPE_FLOAT32:
    for (yy = 0; yy < imOut->ysize; yy++) {
      ymin = bounds[yy * 2 + 0];
      ymax = bounds[yy * 2 + 1];
      k = &kk[yy * ksize];
      for (xx = 0; xx < imOut->xsize; xx++) {
        ss = 0.0;
        for (y = 0; y < ymax; y++) {
          ss += IMAGING_PIXEL_F(imIn, xx, y + ymin) * k[y];
        }
        IMAGING_PIXEL_F(imOut, xx, yy) = ss;
      }
    }
    break;
  }
}

typedef void (*ResampleFunction)(Imaging imOut, Imaging imIn, int offset,
                                 int ksize, int *bounds, double *kk);

Imaging ImagingResampleInner(Imaging imIn, int xsize, int ysize,
                             struct filter *filterp, float box[4],
                             ResampleFunction ResampleHorizontal,
                             ResampleFunction ResampleVertical);

Imaging ImagingResample(Imaging imIn, int xsize, int ysize, int filter,
                        float box[4]) {
  struct filter *filterp;
  ResampleFunction ResampleHorizontal;
  ResampleFunction ResampleVertical;

  if (strcmp(imIn->mode, "P") == 0 || strcmp(imIn->mode, "1") == 0) {
    return (Imaging)ImagingError_ModeError();
  }

  if (imIn->type == IMAGING_TYPE_SPECIAL) {
    return (Imaging)ImagingError_ModeError();
  } else if (imIn->image8) {
    ResampleHorizontal = ImagingResampleHorizontal_8bpc;
    ResampleVertical = ImagingResampleVertical_8bpc;
  } else {
    switch (imIn->type) {
    case IMAGING_TYPE_UINT8:
      ResampleHorizontal = ImagingResampleHorizontal_8bpc;
      ResampleVertical = ImagingResampleVertical_8bpc;
      break;
    case IMAGING_TYPE_INT32:
    case IMAGING_TYPE_FLOAT32:
      ResampleHorizontal = ImagingResampleHorizontal_32bpc;
      ResampleVertical = ImagingResampleVertical_32bpc;
      break;
    default:
      return (Imaging)ImagingError_ModeError();
    }
  }

  /* check filter */
  switch (filter) {
  case IMAGING_TRANSFORM_BOX:
    filterp = &BOX;
    break;
  case IMAGING_TRANSFORM_BILINEAR:
    filterp = &BILINEAR;
    break;
  case IMAGING_TRANSFORM_HAMMING:
    filterp = &HAMMING;
    break;
  case IMAGING_TRANSFORM_BICUBIC:
    filterp = &BICUBIC;
    break;
  case IMAGING_TRANSFORM_LANCZOS:
    filterp = &LANCZOS;
    break;
  default:
    return (Imaging)ImagingError_ValueError("unsupported resampling filter");
  }

  return ImagingResampleInner(imIn, xsize, ysize, filterp, box,
                              ResampleHorizontal, ResampleVertical);
}

Imaging ImagingResampleInner(Imaging imIn, int xsize, int ysize,
                             struct filter *filterp, float box[4],
                             ResampleFunction ResampleHorizontal,
                             ResampleFunction ResampleVertical) {
  Imaging imTemp = NULL;
  Imaging imOut = NULL;

  int i, need_horizontal, need_vertical;
  int ybox_first, ybox_last;
  int ksize_horiz, ksize_vert;
  int *bounds_horiz, *bounds_vert;
  double *kk_horiz, *kk_vert;

  need_horizontal = xsize != imIn->xsize || box[0] || box[2] != xsize;
  need_vertical = ysize != imIn->ysize || box[1] || box[3] != ysize;

  ksize_horiz = precompute_coeffs(imIn->xsize, box[0], box[2], xsize, filterp,
                                  &bounds_horiz, &kk_horiz);
  if (!ksize_horiz) {
    return NULL;
  }

  ksize_vert = precompute_coeffs(imIn->ysize, box[1], box[3], ysize, filterp,
                                 &bounds_vert, &kk_vert);
  if (!ksize_vert) {
    free(bounds_horiz);
    free(kk_horiz);
    return NULL;
  }

  // First used row in the source image
  ybox_first = bounds_vert[0];
  // Last used row in the source image
  ybox_last = bounds_vert[ysize * 2 - 2] + bounds_vert[ysize * 2 - 1];

  /* two-pass resize, horizontal pass */
  if (need_horizontal) {
    // Shift bounds for vertical pass
    for (i = 0; i < ysize; i++) {
      bounds_vert[i * 2] -= ybox_first;
    }

    imTemp = ImagingNewDirty(imIn->mode, xsize, ybox_last - ybox_first);
    if (imTemp) {
      ResampleHorizontal(imTemp, imIn, ybox_first, ksize_horiz, bounds_horiz,
                         kk_horiz);
    }
    free(bounds_horiz);
    free(kk_horiz);
    if (!imTemp) {
      free(bounds_vert);
      free(kk_vert);
      return NULL;
    }
    imOut = imIn = imTemp;
  } else {
    // Free in any case
    free(bounds_horiz);
    free(kk_horiz);
  }

  /* vertical pass */
  if (need_vertical) {
    imOut = ImagingNewDirty(imIn->mode, imIn->xsize, ysize);
    if (imOut) {
      /* imIn can be the original image or horizontally resampled one */
      ResampleVertical(imOut, imIn, 0, ksize_vert, bounds_vert, kk_vert);
    }
    /* it's safe to call ImagingDelete with empty value
       if previous step was not performed. */
    ImagingDelete(imTemp);
    free(bounds_vert);
    free(kk_vert);
    if (!imOut) {
      return NULL;
    }
  } else {
    // Free in any case
    free(bounds_vert);
    free(kk_vert);
  }

  /* none of the previous steps are performed, copying */
  if (!imOut) {
    imOut = ImagingCopy(imIn);
  }

  return imOut;
}
