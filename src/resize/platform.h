/*
 * The Python Imaging Library
 * $Id$
 *
 * platform declarations for the imaging core library
 *
 * Copyright (c) Fredrik Lundh 1995-2003.
 */

// --- From Pillow ImPlatform.h ---

#if defined(PIL_NO_INLINE)
#define inline
#else
#if defined(_MSC_VER) && !defined(__GNUC__)
#define inline __inline
#endif
#endif

#if defined(_WIN32) || defined(__CYGWIN__) /* WIN */

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#ifdef __CYGWIN__
#undef _WIN64
#undef _WIN32
#undef __WIN32__
#undef WIN32
#endif

#else /* not WIN */
/* For System that are not Windows, we'll need to define these. */
/* We have to define them instead of using typedef because the JPEG lib also
   defines their own types with the same names, so we need to be able to undef
   ours before including the JPEG code. */

#if __STDC_VERSION__ >= 199901L /* C99+ */

#include <stdint.h>

#define INT8 int8_t
#define UINT8 uint8_t
#define INT16 int16_t
#define UINT16 uint16_t
#define INT32 int32_t
#define UINT32 uint32_t

#else /* < C99 */

#define INT8 signed char

#if SIZEOF_SHORT == 2
#define INT16 short
#elif SIZEOF_INT == 2
#define INT16 int
#else
#error Cannot find required 16-bit integer type
#endif

#if SIZEOF_SHORT == 4
#define INT32 short
#elif SIZEOF_INT == 4
#define INT32 int
#elif SIZEOF_LONG == 4
#define INT32 long
#else
#error Cannot find required 32-bit integer type
#endif

#define UINT8 unsigned char
#define UINT16 unsigned INT16
#define UINT32 unsigned INT32

#endif /* < C99 */

#endif /* not WIN */

/* assume IEEE; tweak if necessary (patches are welcome) */
#define FLOAT16 UINT16
#define FLOAT32 float
#define FLOAT64 double

#ifdef _MSC_VER
typedef signed __int64 int64_t;
#endif

#ifdef __GNUC__
#define GCC_VERSION                                                            \
  (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#endif

#ifdef WORDS_BIGENDIAN
#define MAKE_UINT32(u0, u1, u2, u3)                                            \
  ((UINT32)(u3) | ((UINT32)(u2) << 8) | ((UINT32)(u1) << 16) |                 \
   ((UINT32)(u0) << 24))
#define MASK_UINT32_CHANNEL_0 0xff000000
#define MASK_UINT32_CHANNEL_1 0x00ff0000
#define MASK_UINT32_CHANNEL_2 0x0000ff00
#define MASK_UINT32_CHANNEL_3 0x000000ff
#else
#define MAKE_UINT32(u0, u1, u2, u3)                                            \
  ((UINT32)(u0) | ((UINT32)(u1) << 8) | ((UINT32)(u2) << 16) |                 \
   ((UINT32)(u3) << 24))
#define MASK_UINT32_CHANNEL_0 0x000000ff
#define MASK_UINT32_CHANNEL_1 0x0000ff00
#define MASK_UINT32_CHANNEL_2 0x00ff0000
#define MASK_UINT32_CHANNEL_3 0xff000000
#endif

#define SHIFTFORDIV255(a) ((((a) >> 8) + a) >> 8)

/* like (a * b + 127) / 255), but much faster on most platforms */
#define MULDIV255(a, b, tmp) (tmp = (a) * (b) + 128, SHIFTFORDIV255(tmp))

#define DIV255(a, tmp) (tmp = (a) + 128, SHIFTFORDIV255(tmp))

#define BLEND(mask, in1, in2, tmp1)                                            \
  DIV255(in1 * (255 - mask) + in2 * mask, tmp1)

#define PREBLEND(mask, in1, in2, tmp1)                                         \
  (MULDIV255(in1, (255 - mask), tmp1) + in2)

#define CLIP8(v) ((v) <= 0 ? 0 : (v) < 256 ? (v) : 255)

/* This is to work around a bug in GCC prior 4.9 in 64 bit mode.
   GCC generates code with partial dependency which is 3 times slower.
   See: https://stackoverflow.com/a/26588074/253146 */
#if defined(__x86_64__) && defined(__SSE__) && !defined(__NO_INLINE__) &&      \
    !defined(__clang__) && defined(GCC_VERSION) && (GCC_VERSION < 40900)
static float __attribute__((always_inline)) inline _i2f(int v) {
  float x;
  __asm__("xorps %0, %0; cvtsi2ss %1, %0" : "=x"(x) : "r"(v));
  return x;
}
#else
static float inline _i2f(int v) { return (float)v; }
#endif
