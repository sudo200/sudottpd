#include "ls.h"

int sorter(const void *a, const void *b)
{
  return strcasecmp(*(const char **)a, *(const char **)b);
}

string_array_t ls(DIR *dir)
{
  if(dir == NULL)
    return (string_array_t) {};

  long current_pos = telldir(dir);
  rewinddir(dir);

  struct dirent *entry;
  string_array_t arr = {
    .start = NULL,
    .len = 0,
  };
  while((entry = readdir(dir)) != NULL)
  {
    arr.start = (char **) realloc(arr.start, sizeof(*arr.start) * ++(arr.len));
    arr.start[arr.len - 1] = strdup(entry->d_name);
  }
  
  seekdir(dir, current_pos);
  
  qsort(arr.start, arr.len, sizeof(*arr.start), sorter);
  return arr;
}

