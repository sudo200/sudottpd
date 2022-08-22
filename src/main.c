#include <math.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pthread.h>

#include "server.h"
#include "http.h"
#include "http_utils.h"
#include "http_server.h"

static volatile http_server_t server;

void * worker(void *);

// ------------------------
const char * content_type = "text/html";
const char * response = "<!DOCTYPE html>"
			"<html lang=\"en\">"
			"<head>"
			"<title>Hello there from C!</title>"
			"<style>"
			"html {"
			"background-color: black;"
			"color: whitesmoke;"
			"font-family: sans-serif;"
			"}"
			"</style>"
			"</head>"
			"<body>"
			"<h1>Hello there from C!</h1>"
			"</body>"
			"</html>";
// ------------------------

// {{{
// SIGINT-Handler
void sigint_handler(int ignore)
{
  puts("\nReceived SIGINT, shutting down...");
  fclose(server.log);
  shutdown(server.serverfd, SHUT_RDWR);
  close(server.serverfd);
  exit(EXIT_SUCCESS);
}
// }}}

// {{{
// Main
int main(int argc, char **argv)
{
  const struct sigaction sigint = {
    .sa_handler = sigint_handler,
  };
  sigaction(SIGINT, &sigint, NULL);

  server.log = stdout;

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
    uint32_t ip_addr = *(uint32_t *) &addr.sin_addr;
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

// {{{
// Worker
void * worker(void *p)
{
  FILE * connection = (FILE *) p;
  fd_t confd = fileno(connection);

  http_request_t req = {};

  int code = parse_http_request(connection, &req);

  if(req.url) free((void *) req.url);
  if(req.headers) free(req.headers);
  if(req.payload) free(req.payload);

  fprintf(server.log, "[WORKER] Incoming request! Sending response...\n");

  http_response_t res = {};

  size_t content_length = strlen(response);
  char cl_str[(int)((ceil(log10(content_length)) + 1 ) * sizeof(char))];
  sprintf(cl_str, "%lu", content_length);

  res.http_version = HTTP_1_1;
  res.status = OK;
  http_header_t headers[] = {
    get_date_header(),
    { .name="Content-Type", .value=content_type },
    { .name="Server", .value="C" },
    { .name="Content-Length", .value=cl_str },
    { .name="Connection", .value="close" },
  };
  res.headers = headers;
  res.headers_len = 5;

  res.payload = (void *) response;
  res.payload_len = content_length;

  stringify_http_response(confd, res);

  fclose(connection);
  fprintf(server.log, "[WORKER] Response sent!\n");

  return NULL;
}
// }}}

