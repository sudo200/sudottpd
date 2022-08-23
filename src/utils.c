#include "utils.h"

int fpeek(FILE *stream)
{
    int c;

    c = fgetc(stream);
    ungetc(c, stream);

    return c;
}

char * fpeeks(FILE * stream, size_t n)
{
  if(stream == NULL)
    return NULL;

  char *buffer = (char *) malloc(n + 1);
  if(buffer == NULL)
    return NULL;

  for(size_t i = 0; i < n; i++)
    buffer[i] = fgetc(stream);
  buffer[n] = '\0';

  for(size_t i = n; i > 0; i--)
    ungetc(buffer[i - 1], stream);

  return buffer;
}

int fpeekstrcmp(FILE * stream, const char * str)
{
  if(str == NULL || stream == NULL)
    return -1;

  const char * peek = fpeeks(stream, strlen(str));
  if(peek == NULL)
    return -1;

  int res = strcmp(peek, str);
  free((void *) peek);
  return (res == 0);
}

size_t fsize(FILE * stream)
{
  fpos_t pos;
  size_t start = 0, end = 0;
  fgetpos(stream, &pos);
  fseek(stream, 0L, SEEK_SET);
  start = ftell(stream);
  fseek(stream, 0L, SEEK_END);
  end = ftell(stream);
  fsetpos(stream, &pos);
  return end - start;
}

