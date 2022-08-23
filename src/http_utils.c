#include "http_utils.h"

#undef  ARRAYLEN
#define ARRAYLEN(arr) (sizeof(arr) / sizeof(*arr))

static const char *day2string[] = {
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat"
};

static char buffer[31];

http_header_t get_date_header()
{
  time_t t;
  struct tm pt = {};
  time(&t);
  gmtime_r(&t, &pt);
  sprintf(buffer, "%s, %02d %02d %04d %02d:%02d:%02d GMT",
      day2string[pt.tm_wday],
      pt.tm_mday,
      pt.tm_mon,
      pt.tm_year + 1900,
      pt.tm_hour % 24,
      pt.tm_min,
      pt.tm_sec
      );
  return (http_header_t) {
    .name = "Date", .value = buffer
  };
}

int send_html_response
(fd_t out, http_version_t version, http_status_code_t status, const char *html)
{
  if(html == NULL || out < 0)
    return -1;

  size_t content_length = strlen(html);
  char cl_str[(int)((ceil(log10(content_length)) + 1 ) * sizeof(char))];
  sprintf(cl_str, "%lu", content_length);

  http_header_t headers[] = {
    get_date_header(),
    { .name="Content-Type", .value="text/html; charset=ascii" },
    { .name="Server", .value="C" },
    { .name="Content-Length", .value=cl_str },
    { .name="Connection", .value="close" },
  };

  http_response_t res = {
    .http_version = version,
    .status = status,

    .headers = headers,
    .headers_len = ARRAYLEN(headers),

    .payload = (void *) html,
    .payload_len = content_length,
  };
  return send_http_response(out, res);
}

int send_file_response
(fd_t out, http_version_t version, http_status_code_t status, const char *path)
{
  if(path == NULL || out < 0)
    return -1;

  FILE * file = fopen(path, "r");
  if(file == NULL)
    return -3;

  size_t content_length = fsize(file);
  char cl_str[(int)((ceil(log10(content_length)) + 1 ) * sizeof(char))];
  sprintf(cl_str, "%lu", content_length);

  const char *mime_type;
  char *extention = strrchr(path, '.');
  if(extention == NULL || (mime_type = mime_get(extention)) == NULL)
    mime_type = mime_default;

  http_header_t headers[] = {
    get_date_header(),
    { .name="Content-Type", .value=mime_type },
    { .name="Server", .value="C" },
    { .name="Content-Length", .value=cl_str },
    { .name="Connection", .value="close" },
  };

  http_response_t res = {
    .http_version = version,
    .status = status,

    .headers = headers,
    .headers_len = ARRAYLEN(headers),

    .payload = malloc(content_length),
    .payload_len = content_length,
  };

  if(res.payload == NULL) {
    fclose(file);
    return -2;
  }

  if(fread(res.payload, 1, content_length, file) != content_length)
  {
    fclose(file);
    return -1;
  }
  fclose(file);

  int ret = 0;
  if(send_http_response(out, res) < 0)
    ret = -1;

  free(res.payload);
  return ret;
}

