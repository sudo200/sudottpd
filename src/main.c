#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include "mime.h"
#include "server.h"
#include "http.h"
#include "http_utils.h"
#include "http_server.h"

#define BAD_REQUEST(fd) send_html_response(fd, HTTP_1_1, BAD_REQUEST, "<!DOCTYPE html>"\
    "<html lang=\"en\">"\
    "<head>"\
    "<title>400 Bad Request</title>"\
    "</head>"\
    "<body>"\
    "<h1>400 Bad Request</h1>"\
    "<hr>"\
    "</body>"\
    "</html>"\
    )

#define INTERNAL_SERVER_ERROR(fd) send_html_response(fd, HTTP_1_1, INTERNAL_SERVER_ERROR,\
    "<!DOCTYPE html>"\
    "<html lang=\"en\">"\
    "<head>"\
    "<title>500 Internal Sever Error</title>"\
    "</head>"\
    "<body>"\
    "<h1>500 Internal Server Error</h1>"\
    "<hr>"\
    "</body>"\
    "</html>"\
    )

#define INSUFFICIENT_STORAGE(fd)  send_html_response(fd, HTTP_1_1, INSUFFICIENT_STORAGE,\
    "<!DOCTYPE html>"\
    "<html lang=\"en\">"\
    "<head>"\
    "<title>507 Insufficient Storage</title>"\
    "</head>"\
    "<body>"\
    "<h1>507 Insufficient Storage</h1>"\
    "<hr>"\
    "</body>"\
    "</html>"\
    )

#define NOT_FOUND(fd) send_html_response(fd, HTTP_1_1, NOT_FOUND,\
    "<!DOCTYPE html>"\
    "<html lang=\"en\">"\
    "<head>"\
    "<title>404 Not Found</title>"\
    "</head>"\
    "<body>"\
    "<h1>404 Not Found</h1>"\
    "<hr>"\
    "</body>"\
    "</html>"\
    )

static volatile http_server_t server;

void * worker(void *);

// {{{ SIGINT-Handler
void sigint_handler(int ignore)
{
  puts("\nReceived SIGINT, shutting down...");
  fclose(server.log);
  shutdown(server.serverfd, SHUT_RDWR);
  close(server.serverfd);
  mime_unload();
  exit(EXIT_SUCCESS);
}
// }}}

// {{{ Main
int main(int argc, char **argv)
{
  const struct sigaction sigint = {
    .sa_handler = sigint_handler,
  };
  sigaction(SIGINT, &sigint, NULL);

  server.log = stdout;

  FILE * mime_types = fopen("mime.types", "r");
  if(mime_types == NULL)
  {
    fprintf(server.log, "[MAIN] Couldn't open mime.types file: %m\n");
    return EXIT_FAILURE;
  }
  mime_load(mime_types);
  fclose(mime_types);

  const uint8_t server_addr[4] = {127,0,0,1};
  server.serverfd = mkserver_inet(server_addr, 8080, 0xFF);

  if(server.serverfd < 0)
  {
    fprintf(server.log, "[MAIN] Couldn't create server: %m\n");
    return EXIT_FAILURE;
  }

  fprintf(server.log, "[MAIN] Listening on port 8080...\n");

  while(1)
  {
    pthread_t thread;
    struct sockaddr_in addr = {};
    socklen_t addr_len = sizeof(addr);

    fd_t confd = accept(server.serverfd, (struct sockaddr *) &addr, &addr_len);
    if(confd < 0)
    {
      fprintf(server.log, "[MAIN] Error handling connection: %m\n");
      continue;
    }
    uint32_t ip_addr = *(uint32_t *)&addr.sin_addr;
    
    fprintf(server.log, "[MAIN] Incoming connection from %u.%u.%u.%u:%u!\n",
        ip_addr & 0x000000FF,
        (ip_addr & 0x0000FF00) >> 8,
        (ip_addr & 0x00FF0000) >> 16,
        (ip_addr & 0xFF000000) >> 24,
        ntohs(addr.sin_port));

    FILE * connection = fdopen(confd, "rw");
    pthread_create(&thread, NULL, worker, (void *) connection);
    pthread_join(thread, NULL);
  }

  return EXIT_SUCCESS;
}
// }}}

// {{{ Worker
void * worker(void *p)
{
  FILE * connection = (FILE *) p;
  fd_t confd = fileno(connection);

  http_request_t req = {};

  if(parse_http_request(connection, &req) < 0)
  {
    BAD_REQUEST(confd);
    return NULL;
  }

  const char * rootdir = "public";
  char * full_path = (char *) malloc(strlen(rootdir)+strlen(req.url)+1);
  if(full_path == NULL)
  {
    INSUFFICIENT_STORAGE(confd);
    goto finish;
  }
  strcpy(full_path, rootdir);
  strcat(full_path, req.url);

  fprintf(server.log, "[WORKER] Incoming request! Sending response...\n");

  switch(send_file_response(confd, HTTP_1_1, OK, full_path))
  {
    case -1: // Internal Server Error
      INTERNAL_SERVER_ERROR(confd);
      break;

    case -2: // Insufficient Storage
      INSUFFICIENT_STORAGE(confd);
      break;

    case -3: // Not Found
      NOT_FOUND(confd);
      break;

    default:
      break;
  }

  free(full_path);
finish:
  if(req.url) free((void *) req.url);
  for(size_t i = 0; i < req.headers_len; i++) {
    free((void *) req.headers[i].name);
    free((void *) req.headers[i].value);
  }
  if(req.headers) free(req.headers);
  if(req.payload) free(req.payload);
  fclose(connection);
  fprintf(server.log, "[WORKER] Response sent!\n");

  return NULL;
}
// }}}

