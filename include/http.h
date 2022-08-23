#ifndef ___HTTP_H__
#define ___HTTP_H__

#include <errno.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>

#include <sys/file.h>

#include "utils.h"

typedef int fd_t;

typedef enum
{
  CONTINUE = 100,
  SWITCHING_PROTOCOLS = 101,
  PROCESSING = 102,
  EARLY_HINTS = 103,

  OK = 200,
  CREATED = 201,
  ACCEPTED = 202,
  NON_AUTHORATIVE_INFORMATION = 203,
  NO_CONTENT = 204,
  RESET_CONTENT = 205,
  PARTIAL_CONTENT = 206,
  MULTI_STATUS = 207,
  ALREADY_EXPORTED = 208,
  IM_USED = 226,

  MULTIPLE_CHOICES = 300,
  MOVED_PERMANENTLY = 301,
  FOUND = 302,
  SEE_OTHER = 303,
  NOT_MODIFIED = 304,
  USE_PROXY = 305,
  SWITCH_PROXY = 306,
  TEMPORARY_REDIRECT = 307,
  PERMANENT_REDIRECT = 308,

  BAD_REQUEST = 400,
  UNAUTHORIZED = 401,
  PAYMENT_REQUIRED = 402,
  FORBIDDEN = 403,
  NOT_FOUND = 404,
  METHOD_NOT_ALLOWED = 405,
  NOT_ACCEPTABLE = 406,
  PROXY_AUTHENTICATION_REQUIRED = 407,
  REQUEST_TIMEOUT = 408,
  CONFLICT = 409,
  GONE = 410,
  LENGTH_REQUIRED = 411,
  PRECONDITION_FAILED = 412,
  PAYLOAD_TOO_LARGE = 413,
  URI_TOO_LONG = 414,
  UNSUPPORTED_MEDIA_TYPE = 415,
  RANGE_NOT_SATISFIABLE = 416,
  EXPECTATION_FAILED = 417,
  IM_A_TEAPOT = 418,
  MISDIRECTED_REQUEST = 421,
  UNPROCESSABLE_ENTITY = 422,
  LOCKED = 423,
  FAILED_DEPENDENCY = 424,
  TOO_EARLY = 425,
  UPGRADE_REQUIRED = 426,
  PRECONDITION_REQUIRED = 428,
  TOO_MANY_REQUESTS = 429,
  REQUEST_HEADER_FIELDS_TOO_LARGE = 431,
  UNAVAILABLE_FOR_LEGAL_REASONS = 451,

  INTERNAL_SERVER_ERROR = 500,
  NOT_IMPLEMENTED = 501,
  BAD_GATEWAY = 502,
  SERVICE_UNAVAILABLE = 503,
  GATEWAY_TIMEOUT = 504,
  HTTP_VERSION_NOT_SUPPORTED = 505,
  VARIANT_ALSO_NEGOTIATES = 506,
  INSUFFICIENT_STORAGE = 507,
  LOOP_DETECTED = 508,
  NOT_EXTENDED = 510,
  NETWORK_AUTHENTICATION_REQUIRED = 511,
}
http_status_code_t;

typedef enum
{
  UNKNOWN_METHOD = -1,

  GET,
  HEAD,
  POST,
  PUT,
  DELETE,
  CONNECT,
  OPTIONS,
  TRACE,
  PATCH
}
http_method_t;

const char * http_method_to_string(http_method_t method);

http_method_t http_method_from_string(const char * str);

typedef enum
{
  UNKNOWN_VERSION = -1,

  HTTP_1_0,
  HTTP_1_1,
  HTTP_2_0,
}
http_version_t;

const char * http_version_to_string(http_version_t version);

http_version_t http_version_from_string(const char * str);

typedef struct
{
  const char *name;
  const char *value;
}
http_header_t;

const char * get_http_header_value
(const http_header_t * headers, size_t headers_len, const char * name);

typedef struct
{
  http_method_t method;
  const char * url;
  http_version_t http_version;

  http_header_t * headers;
  size_t headers_len;

  void * payload;
  size_t payload_len;
}
http_request_t;

typedef struct
{
  http_version_t http_version;
  http_status_code_t status;

  http_header_t * headers;
  size_t headers_len;

  void * payload;
  size_t payload_len;
}
http_response_t;

int parse_http_request(FILE * in, http_request_t *req);
int send_http_request(fd_t out, http_request_t req);

int parse_http_response(FILE * in, http_response_t *res);
int send_http_response(fd_t out, http_response_t res);

#endif//___HTTP_H__

