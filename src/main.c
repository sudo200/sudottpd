#include <errno.h>
#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <pthread.h>

#include "ls.h"
#include "mime.h"
#include "server.h"
#include "http.h"
#include "http_utils.h"
#include "http_server.h"
#include "utils.h"

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

#define METHOD_NOT_ALLOWED(fd)  send_html_response(fd, HTTP_1_1, METHOD_NOT_ALLOWED,\
    "<!DOCTYPE html>"\
    "<html lang=\"en\">"\
    "<head>"\
    "<title>405 Method Not Allowed</title>"\
    "</head>"\
    "<body>"\
    "<h1>405 Method Not Allowed</h1>"\
    "</body>"\
    "</html>"\
    )

static volatile http_server_t server;

const char * rootdir = "public";

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

  if(mkdir(rootdir, 0777) < 0 && errno != EEXIST)
  {
    fprintf(server.log, "[MAIN] Couldn't create directory \"%s\": %m\n", rootdir);
    return EXIT_FAILURE;
  }

  const uint8_t server_addr[4] = {127,0,0,1};
  server.serverfd = mkserver_inet(NULL, 8080, 0xFF);

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

    fd_t *confd = (fd_t *) malloc(sizeof(*confd));
    *confd = accept(server.serverfd, (struct sockaddr *) &addr, &addr_len);
    if(*confd < 0)
    {
      fprintf(server.log, "[MAIN] Error handling connection: %m\n");
      free(confd);
      continue;
    }
    uint32_t ip_addr = *(uint32_t *)&addr.sin_addr;
    
    fprintf(server.log, "[MAIN] Incoming connection from %u.%u.%u.%u:%u!\n",
        ip_addr & 0x000000FF,
        (ip_addr & 0x0000FF00) >> 8,
        (ip_addr & 0x00FF0000) >> 16,
        (ip_addr & 0xFF000000) >> 24,
        ntohs(addr.sin_port));

    pthread_create(&thread, NULL, worker, (void *) confd);
    pthread_detach(thread);
  }

  return EXIT_SUCCESS;
}
// }}}

// {{{ Worker
void * worker(void *p)
{
  FILE * connection = fdopen(*(fd_t *) p, "r");
  fd_t confd = *(fd_t *) p;
  free(p);

  http_request_t req = {};

  if(parse_http_request(connection, &req) < 0)
  {
    BAD_REQUEST(confd);
    return NULL;
  }

  char *path = (char *) req.url, *query;
  if((query = strchr(req.url, '?')) != NULL)
    *query++ = '\0';

  if(req.method != GET)
  {
    METHOD_NOT_ALLOWED(confd);
    goto finish;
  }

  char * full_path = (char *) malloc(strlen(rootdir)+strlen(path)+1);
  if(full_path == NULL)
  {
    INSUFFICIENT_STORAGE(confd);
    goto finish;
  }
  strcpy(full_path, rootdir);
  strcat(full_path, path);

  fprintf(server.log, "[WORKER] Incoming request! Sending response...\n");

  DIR *dir;
  if((dir = opendir(full_path)) == NULL)
    switch(errno)
    {
      case ENOTDIR: // File
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
        break;

      case ENOENT: // Not found
        NOT_FOUND(confd);
        break;

      default: // Other errors
        INTERNAL_SERVER_ERROR(confd);
        break;
    }
  else // Directory
  {
    string_array_t directory = ls(dir);

    char *list, *path_cpy = strdup(path);
    *(strrchr(path_cpy, '/') + 1) = '\0';
    asprintf(&list, "<li><a href=\"%1$s\">Parent Directory</a></li>", path_cpy);

    const char *format = (strcmp(path, "/") == 0)                                   
      ? "<li><a href=\"%2$s%1$s\">%1$s</a></li>"  
      : "<li><a href=\"%2$s/%1$s\">%1$s</a></li>";

    for(size_t i = 2; i < directory.len; i++)
    {
      char *tmp;
      asprintf(&tmp, format, directory.start[i], path);
      strcata(&list, tmp);
      free(tmp);
    }

    char *body;
    asprintf(&body,
        "<!DOCTYPE html>"
        "<html lang=\"en\">"
        "<head>"
        "<title>Directory listing of %1$s</title>"
        "</head>"
        "<body>"
        "<h1>Directory listing of %1$s</h1>"
        "<hr>"
        "<ul>%2$s</ul>"
        "</body>"
        "</html>",
        path,
        list
        );
    free(path_cpy);
    free(list);

    send_html_response(confd, HTTP_1_1, OK, body);

    free(body);
    for(size_t i = 0; i < directory.len; i++)
      free(directory.start[i]);
    free(directory.start);
  }

finish:
  if(dir != NULL)
    closedir(dir);
  free(full_path);
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

