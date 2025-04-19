#include <dirent.h>
#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

// for easier handling of file entries, less array mess, etc
typedef struct Entry {
  char *filename;
  off_t filesize;
} Entry;

Entry *newEntry(const char *filename, off_t filesize) {
  Entry *e = malloc(sizeof(Entry));

  if (!e) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  e->filename = strdup(filename);
  e->filesize = filesize;

  return e;
}

void freeEntry(Entry *e) {
  if (e) {
    free(e->filename);
    free(e);
  }
}

long long read_directory(const char *path, int depth) {
  DIR *dir = opendir(path);
  if (dir == NULL) {
    fprintf(stderr, "Couldn't open directory '%s': %s\n", path,
            strerror(errno));
    return 0;
  }

  struct dirent *entry;
  struct stat st;
  long long bytes = 0;

  // Indent and print the directory label
  for (int i = 0; i < depth * 2; i++)
    printf(" ");
  printf("dir %s\n", path);

  // print files
  rewinddir(dir); // reset
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    char fullpath[PATH_MAX];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

    if (lstat(fullpath, &st) == -1) {
      perror("lstat");
      continue;
    }

    if (S_ISREG(st.st_mode)) {
      Entry *e = newEntry(entry->d_name, st.st_size);
      for (int i = 0; i < (depth + 1) * 2; i++)
        printf(" ");
      printf("%7lld:%s\n", (long long)e->filesize, e->filename);
      bytes += e->filesize;
      freeEntry(e);
    }
  }

  // recurse
  rewinddir(dir); // reset
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    char fullpath[PATH_MAX];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

    if (lstat(fullpath, &st) == -1) {
      perror("lstat");
      continue;
    }

    if (S_ISDIR(st.st_mode)) {
      bytes += read_directory(fullpath, depth + 1);
    }
  }

  closedir(dir);
  return bytes;
}

int main(int argc, char *argv[]) {

  const char *start;

  if (argc > 1) {
    start = argv[1];
  } else {
    start = ".";
  }

  long long total = read_directory(start, 0);

  printf("\nTotal bytes used by files: %lld\n", total);

  return 0;

  // open a directory
  // parse each directory entry
  // display file size and file name
  // recursively open subdiretories

  // if no argument is provided, then open the current working directory
  // if argument is provided, it will open directory provided

  // need the following APIs:
  /*

  DIR *opendir(const char *name);
  struct dirent *readdir(DIR *dirp);
  int lstat(const char *path, struct stat *buf);

  */

  // keep a total of bytes used by each file, display at the end of the program
  // run
}
