#ifndef __TESTS_H
#define __TESTS_H

#define TEST(func)                                    \
  if (func() == -1) { printf("[FAIL] " #func "\n"); } \
  else              { printf("[PASS] " #func "\n"); }

#endif // __TESTS_H

