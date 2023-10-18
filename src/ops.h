#include "types.h"

// Clip values from 0 to size boundaries.
dpos_t _clip2size(dpos_t a, ipos_t size);

ipos_t get_scaled_size(ipos_t size, double scaling);

dpos_t _floor(dpos_t a);
dpos_t _ceil(dpos_t a);

dpos_t _add(dpos_t a, double b);
dpos_t _sub(dpos_t a, double b);
dpos_t _mul(dpos_t a, double b);
dpos_t _div(dpos_t a, double b);

dpos_t _addv(dpos_t a, dpos_t b);
dpos_t _subv(dpos_t a, dpos_t b);
