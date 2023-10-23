#include "slide.h"
#include "types.h"

int main(int argc, char **argv) {
  // if (argc != 2) {
  //   printf("Usage: wsi-tvx path/to/slide \n");
  //   return 1;
  // }

  // // Get path to slide
  // char *path = argv[1];
  // printf("path: %s\r\n", path);

  if (argc != 1) {
    printf("Usage: wsi-tvx \n");
    return 1;
  }

  char *path = "/data/slides/Aperio/CMU-1.svs\0";
  printf("path: %s\r\n", path);

  // Load all props
  oslide_t oslide = open_oslide(path);
  print_slide(&oslide);

  // NOTE: Load thumbnail, memory is allocated inside
  for (int i = 0; i < 3; i++) {
    AssociatedImage a = i;
    image_t thumbnail = {.height = 0, .width = 0, .bands = 4, .data = NULL};
    get_thumbnail(oslide.osr, &thumbnail, a);
    printf("  %10s: %5d, %5d\n", stringFromAssociatedImage(a), thumbnail.width,
           thumbnail.height);
    free(thumbnail.data);
  }

  // Close and free
  close_oslide(&oslide);

  return 0;
}
