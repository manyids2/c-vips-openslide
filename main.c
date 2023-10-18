#include <openslide/openslide.h>
#include <vips/vips.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    printf("Usage: wsi-tvx path/to/slide \n");
    return 1;
  }

  // Get path to slide
  char *slidepath = argv[1];
  printf("slidepath: %s\r\n", slidepath);

  return 0;
}
