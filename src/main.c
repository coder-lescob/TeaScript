#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

char *file_read_all(char *path);

int main(int argc, char **argv) {
  
  if (argc == 1) {
    printf("Usage: %s <file-name>\n", argv[0]);
    return 0;
  }

  char *code = file_read_all(argv[1]);
  if (code == NULL) {
    fprintf(stderr, "could not run %s: %s\n", argv[1], strerror(errno));
    return -1;
  }
  
  printf("%s\n", code);

  free(code);

  return 0;
}

static size_t get_file_len(FILE *fd) {
  // go to the end, ask for the offset from the start go back to the start.
  // this offset is the length of the file.
  fseek(fd, 0, SEEK_END);
  size_t file_len = ftell(fd);
  fseek(fd, 0, SEEK_SET);
  
  return file_len;
}

char *file_read_all(char *path) {
  char *content;

  FILE *fd = fopen(path, "r");
  if (fd == NULL) return NULL; // failed;
  {
    // get the length of the file
    size_t file_len = get_file_len(fd);

    // allocate enough memory, 1 more byte for the null terminator
    content = calloc(file_len + 1, sizeof(char));

    // copy all the file's content to that allocated string
    fread(content, file_len, 1, fd);
  }
  fclose(fd);

  return content;
}
