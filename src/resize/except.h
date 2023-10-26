#include <stdio.h>

void *ImagingError_ValueError(const char *message) {
  if (!message) {
    message = "exception: bad argument to function";
  }
  fprintf(stderr, "*** %s\n", message);
  return NULL;
}

void *ImagingError_Mismatch(void) {
  return ImagingError_ValueError("images don't match");
}

void *ImagingError_ModeError(void) {
  return ImagingError_ValueError("bad image mode");
}

void *ImagingError_OSError(void) {
  fprintf(stderr, "*** exception: file access error\n");
  return NULL;
}

void *ImagingError_MemoryError(void) {
  fprintf(stderr, "*** exception: out of memory\n");
  return NULL;
}

void ImagingError_Clear(void) { /* nop */
  ;
}
