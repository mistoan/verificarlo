#define _GNU_SOURCE
#include <dlfcn.h>
typedef float (*real_cosf)(float);

float cosf(float a) {
  float glibm_result = ((real_cosf)dlsym(RTLD_NEXT, "cosf"))(a);
  return glibm_result + 2.0;
}
