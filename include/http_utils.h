#ifndef ___HTTP_UTILS_H__
#define ___HTTP_UTILS_H__

#include <time.h>
#include <math.h>
#include <string.h>

#include "http.h"
#include "mime.h"

/**
 * Returns a Date: header with the current time
 */
http_header_t get_date_header();

/**
 * Sends html with given http version and status code
 */
int send_html_response
(fd_t out, http_version_t version, http_status_code_t status, const char *html);

int send_file_response
(fd_t out, http_version_t version, http_status_code_t status, const char *path);

#endif//___HTTP_UTILS_H__

