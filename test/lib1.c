#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>

unsigned int (*original_sleep)(unsigned int seconds);

unsigned int sleep(unsigned int seconds)
{
  original_sleep = (unsigned int (*)(unsigned int))dlsym(RTLD_NEXT, "sleep");

  printf("in %s\n", __FILE__);
  return original_sleep(seconds);
}

static void init() __attribute__((constructor));

void init() {
  printf("in init function of %s\n", __FILE__);
}
