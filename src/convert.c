#include "convert.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

/**
 * convert a string to an integer
 * @warning: assumes the string represents entirely an integer.
 */
uint64_t convert_raw_str_to_int(char *str) {
  return strtol(str, NULL, 10);
}

/**
 * convert a string to a float
 * @warning: assumes the string represents entirely a real number
 */
float convert_raw_str_to_float(char *str) {
  return strtod(str, NULL);
}
