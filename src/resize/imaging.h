#include "platform.h"

typedef struct {
  char *ptr;
  int size;
} ImagingMemoryBlock;

typedef struct ImagingMemoryArena {
  int alignment;     /* Alignment in memory of each line of an image */
  int block_size;    /* Preferred block size, bytes */
  int blocks_max;    /* Maximum number of cached blocks */
  int blocks_cached; /* Current number of blocks not associated with images */
  ImagingMemoryBlock *blocks_pool;
  int stats_new_count;        /* Number of new allocated images */
  int stats_allocated_blocks; /* Number of allocated blocks */
  int stats_reused_blocks; /* Number of blocks which were retrieved from a pool
                            */
  int stats_reallocated_blocks; /* Number of blocks which were actually
                                   reallocated after retrieving */
  int stats_freed_blocks;       /* Number of freed blocks */
} *ImagingMemoryArena;

typedef struct ImagingMemoryInstance *Imaging;

/* handle magics (used with PyCObject). */
#define IMAGING_MAGIC "PIL Imaging"

/* pixel types */
#define IMAGING_TYPE_UINT8 0
#define IMAGING_TYPE_INT32 1
#define IMAGING_TYPE_FLOAT32 2
#define IMAGING_TYPE_SPECIAL 3 /* check mode for details */

#define IMAGING_MODE_LENGTH                                                    \
  6 + 1 /* Band names ("1", "L", "P", "RGB", "RGBA", "CMYK", "YCbCr",          \
           "BGR;xy") */

struct ImagingMemoryInstance {
  /* Format */
  char mode[IMAGING_MODE_LENGTH]; /* Band names ("1", "L", "P", "RGB", "RGBA",
                                     "CMYK", "YCbCr", "BGR;xy") */
  int type;                       /* Data type (IMAGING_TYPE_*) */
  int depth;                      /* Depth (ignored in this version) */
  int bands;                      /* Number of bands (1, 2, 3, or 4) */
  int xsize;                      /* Image dimension. */
  int ysize;

  /* Data pointers */
  UINT8 **image8;  /* Set for 8-bit images (pixelsize=1). */
  INT32 **image32; /* Set for 32-bit images (pixelsize=4). */

  /* Internals */
  char **image;               /* Actual raster data. */
  char *block;                /* Set if data is allocated in a single block. */
  ImagingMemoryBlock *blocks; /* Memory blocks for pixel storage */

  int pixelsize; /* Size of a pixel, in bytes (1, 2 or 4) */
  int linesize;  /* Size of a line, in bytes (xsize * pixelsize) */

  /* Virtual methods */
  void (*destroy)(Imaging im);
};

#define IMAGING_PIXEL_1(im, x, y) ((im)->image8[(y)][(x)])
#define IMAGING_PIXEL_L(im, x, y) ((im)->image8[(y)][(x)])
#define IMAGING_PIXEL_LA(im, x, y) ((im)->image[(y)][(x) * 4])
#define IMAGING_PIXEL_P(im, x, y) ((im)->image8[(y)][(x)])
#define IMAGING_PIXEL_PA(im, x, y) ((im)->image[(y)][(x) * 4])
#define IMAGING_PIXEL_I(im, x, y) ((im)->image32[(y)][(x)])
#define IMAGING_PIXEL_F(im, x, y) (((FLOAT32 *)(im)->image32[y])[x])
#define IMAGING_PIXEL_RGB(im, x, y) ((im)->image[(y)][(x) * 4])
#define IMAGING_PIXEL_RGBA(im, x, y) ((im)->image[(y)][(x) * 4])
#define IMAGING_PIXEL_CMYK(im, x, y) ((im)->image[(y)][(x) * 4])
#define IMAGING_PIXEL_YCbCr(im, x, y) ((im)->image[(y)][(x) * 4])

#define IMAGING_PIXEL_UINT8(im, x, y) ((im)->image8[(y)][(x)])
#define IMAGING_PIXEL_INT32(im, x, y) ((im)->image32[(y)][(x)])
#define IMAGING_PIXEL_FLOAT32(im, x, y) (((FLOAT32 *)(im)->image32[y])[x])

extern struct ImagingMemoryArena ImagingDefaultArena;
extern int ImagingMemorySetBlocksMax(ImagingMemoryArena arena, int blocks_max);
extern void ImagingMemoryClearCache(ImagingMemoryArena arena, int new_size);

extern Imaging ImagingNew(const char *mode, int xsize, int ysize);
extern Imaging ImagingNewDirty(const char *mode, int xsize, int ysize);
extern Imaging ImagingNew2Dirty(const char *mode, Imaging imOut, Imaging imIn);
extern void ImagingDelete(Imaging im);

extern Imaging ImagingNewBlock(const char *mode, int xsize, int ysize);

extern Imaging ImagingNewPrologue(const char *mode, int xsize, int ysize);
extern Imaging ImagingNewPrologueSubtype(const char *mode, int xsize, int ysize,
                                         int structure_size);

extern Imaging ImagingCopy(Imaging imIn);
