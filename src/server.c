#include "server.h"
static const uint32_t n = 0;

int mkserver_inet(const uint8_t addr[4], const uint16_t port, const int queue)
{
  int socketfd = socket(AF_INET, SOCK_STREAM, 0);
  if(socketfd < 0)
    return -1;

  struct sockaddr_in adr = {};
  adr.sin_port = htons(port);
  adr.sin_addr = (addr == NULL) ? * (struct in_addr *) &n : * (struct in_addr *) addr;
  adr.sin_family = AF_INET;

  if(bind(socketfd, (struct sockaddr *) &adr, sizeof(adr)) < 0)
    return -1;

  if(listen(socketfd, queue) < 0)
    return -1;

  return socketfd;
}

int mkserver_unix(const char *path, const int queue)
{
  int socketfd = socket(AF_UNIX, SOCK_STREAM, 0);
  if(socketfd < 0)
    return -1;

  struct sockaddr_un adr = {};
  adr.sun_family = AF_UNIX;
  adr.sun_path = path;
  
  if(bind(socketfd, (struct sockaddr *) &adr, sizeof(adr)) < 0)
    return -1;

  if(listen(socketfd, queue) < 0)
    return -1;

  return socketfd;
}

