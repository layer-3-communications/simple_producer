#include "util.h"

/* itoa is not standard. */
char *itoa (int val) {
  static char buf[32] = {0};
  int i = 30;
  int base = 10;

  for (; val && i; --i, val /= base) {
    buf[i] = "0123456789abcdef"[val % base];
  }

  return &buf[i+1];
}
