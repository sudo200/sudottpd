#ifndef ___HTTP_UTILS_H__
#define ___HTTP_UTILS_H__

#include <time.h>

#include "http.h"

/**
 * Returns a Date: header with the current time
 */
http_header_t get_date_header();

#endif//___HTTP_UTILS_H__

