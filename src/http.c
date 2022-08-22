#include "http.h"

#define EQUALS(x, y)  (strcasecmp(x, y) == 0)

http_method_t http_method_from_string(const char * str)
{
  if(EQUALS(str, "GET"))
    return GET;
  if(EQUALS(str, "HEAD"))
    return HEAD;
  if(EQUALS(str, "POST"))
    return POST;
  if(EQUALS(str, "PUT"))
    return PUT;
  if(EQUALS(str, "DELETE"))
    return DELETE;
  if(EQUALS(str, "CONNECT"))
    return CONNECT;
  if(EQUALS(str, "OPTIONS"))
    return OPTIONS;
  if(EQUALS(str, "TRACE"))
    return TRACE;
  if(EQUALS(str, "PATCH"))
    return PATCH;
  return UNKNOWN_METHOD;
}

const char * http_method_to_string(http_method_t method)
{
  switch(method)
  {
    case GET:     return "GET"; 
    case HEAD:    return "HEAD";
    case POST:    return "POST";
    case PUT:     return "PUT";
    case DELETE:  return "DELETE";
    case CONNECT: return "CONNECT";
    case OPTIONS: return "OPTIONS";
    case TRACE:   return "TRACE";
    case PATCH:   return "PATCH";

    default:      return "UNKNOWN_METHOD";
  }
}

const char * http_version_to_string(http_version_t version)       
{
  switch(version)
  {
    case HTTP_1_0:  return "HTTP/1.0";
    case HTTP_1_1:  return "HTTP/1.1";
    case HTTP_2_0:  return "HTTP/2.0";

    default:        return "UNKNOWN_VERSION";
  }
}

http_version_t http_version_from_string(const char * str)
{
  if(EQUALS(str, "HTTP/1.0"))
    return HTTP_1_0;
  if(EQUALS(str, "HTTP/1.1"))
    return HTTP_1_1;
  if(EQUALS(str, "HTTP/2.0"))
    return HTTP_2_0;
  return UNKNOWN_VERSION;
}

const char * get_http_header_value
(const http_header_t * headers, size_t headers_len, const char * name)
{
  for(size_t i = 0; i < headers_len; i++)
    if(EQUALS(name, headers[i].name))
      return headers[i].value;

  return NULL;
}

int parse_http_request(FILE * in, http_request_t *req)
{
  req->payload = NULL;
  req->payload_len = 0;

  char *method, *http_version;
  fscanf(in,
      " %ms %ms %ms",
      &method,
      &req->url,
      &http_version
      ); // Request
  req->method = http_method_from_string(method);
  req->http_version = http_version_from_string(http_version);
  free(method);
  free(http_version);

  http_header_t * headers = NULL;
  size_t headers_len = 0;
  while(!fpeekstrcmp(in, "\r\n\r\n")) // Headers
  {
    headers = (http_header_t *) realloc(headers, sizeof(*headers) * ++headers_len);
    fscanf(in, " %m[^:] : %m[^\r\n]", &(headers + headers_len -1)->name, &(headers + headers_len -1)->value);
  }
  req->headers = headers;
  req->headers_len = headers_len;

  const char * cl_header = get_http_header_value(headers, headers_len, "Content-Length");
  if(cl_header == NULL)
    return 0;

  fscanf(in, " ");

  req->payload_len = strtoll(cl_header, NULL, 0);
  req->payload = malloc(req->payload_len);
  if(req->payload == NULL)
    return -1;

  if(req->payload_len != fread(req->payload, 1, req->payload_len, in))
    return -1;

  return 0;
}

int stringify_http_request(fd_t out, http_request_t req)
{
  // Method, URI and Version
  dprintf(out, "%s %s %s\r\n", http_method_to_string(req.method), req.url, http_version_to_string(req.http_version));

  // Headers
  for(size_t i = 0; i < req.headers_len; i++)
    dprintf(out, "%s: %s\r\n", req.headers[i].name, req.headers[i].value);
  dprintf(out, "\r\n");

  if(req.payload == NULL || req.payload_len == 0)
    return 0;

  // Payload
  if(write(out, req.payload, req.payload_len) == -1)
    return -1;
  return 0;
}

int parse_http_response(FILE * in, http_response_t *res)
{
  res->payload = NULL;
  res->payload_len = 0;

  char *version;
  fscanf(in, " %ms %u %*s", &version, &res->status);
  res->http_version = http_version_from_string(version);
  free(version);

  http_header_t *headers = NULL;
  size_t headers_len = 0;
  while(!fpeekstrcmp(in, "\r\n\r\n")) // Headers
  {
    headers = (http_header_t *) realloc(headers, sizeof(*headers) * ++headers_len);
    fscanf(in, " %m[^:] : %m[^\r\n]", &(headers + headers_len -1)->name, &(headers + headers_len -1)->value);
  }
  res->headers = headers;
  res->headers_len = headers_len;

  const char * cl_header = get_http_header_value(headers, headers_len, "Content-Length");
  if(cl_header == NULL)
    return 0;

  fscanf(in, " ");

  res->payload_len = strtoll(cl_header, NULL, 0);
  res->payload = malloc(res->payload_len);

  if(res->payload_len != fread(res->payload, 1, res->payload_len, in))
    return -1;

  return 0;
}

int stringify_http_response(fd_t out, http_response_t res)
{
  // Version & Status code
  dprintf(out, "%s %u\r\n", http_version_to_string(res.http_version), res.status);

  for(size_t i = 0; i < res.headers_len; i++)
    dprintf(out, "%s: %s\r\n", res.headers[i].name, res.headers[i].value);
  dprintf(out, "\r\n");

  if(res.payload == NULL || res.payload_len == 0)
    return 0;

  // Payload
  if(write(out, res.payload, res.payload_len) == -1)
    return -1;
  return 0;
}

