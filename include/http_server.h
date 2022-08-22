#ifndef ___HTTP_SERVER_H__
#define ___HTTP_SERVER_H__

#include <stdio.h>

#include "http.h"

typedef struct
{
  fd_t serverfd;
  FILE * log;
}
http_server_t;

#endif//___HTTP_SERVER_H__

