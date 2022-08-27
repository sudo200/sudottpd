#ifndef ___LS_H__
#define ___LS_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

#include <sys/types.h>
#include <dirent.h>

typedef struct
{
  char **start;
  size_t len;
}
string_array_t;

string_array_t ls(DIR *dir);

#endif//___LS_H__

