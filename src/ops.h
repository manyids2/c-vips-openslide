#include "types.h"

// Clip values from 0 to size boundaries.
ipos_t clip2size(ipos_t a, ipos_t size);
dpos_t clip2size_d(dpos_t a, dpos_t size);

// Scaling with proper types ( int = double(int) * double )
ipos_t get_scaled_size(ipos_t size, double scaling);

// vector ops
ipos_t _int(dpos_t a);
dpos_t _double(ipos_t a);
dpos_t _floor(dpos_t a);
dpos_t _ceil(dpos_t a);

// (vector, scalar) ops
dpos_t _add(dpos_t a, double b);
dpos_t _sub(dpos_t a, double b);
dpos_t _mul(dpos_t a, double b);
dpos_t _div(dpos_t a, double b);

// (vector, vector) ops
dpos_t _addv(dpos_t a, dpos_t b);
dpos_t _subv(dpos_t a, dpos_t b);

// From openslide-python _convert.c
void argb2rgba(uint32_t *buf, int len);
