#include <stdio.h>
#include <stdlib.h>

int write_csv(char *path, char *text) {
  FILE *f = fopen(path, "w");
  if (f == NULL) {
    printf("Error opening file! : %s\n", path);
    return 0;
  }
  fprintf(f, "Some text: %s\n", text);
  fclose(f);
  return 1;
}

int append_csv(char *path, char *text) {
  FILE *f = fopen(path, "a");
  if (f == NULL) {
    printf("Error opening file! : %s\n", path);
    return 0;
  }
  fprintf(f, "Some text: %s\n", text);
  fclose(f);
  return 1;
}
