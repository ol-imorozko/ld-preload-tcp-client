#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>

/* unsigned int (*lib1_sleep)(unsigned int seconds); */
unsigned int (*libc_sleep)(unsigned int seconds);

/* unsigned int sleep(unsigned int seconds) */
/* { */
/*   lib1_sleep = (unsigned int (*)(unsigned int)) dlsym(RTLD_NEXT, "sleep"); */
/*   original_sleep = (unsigned int (*)(unsigned int)) dlsym(RTLD_DEFAULT, "sleep"); */

/*   if (rand() % 2 == 0) */
/*     return lib1_sleep(seconds); */

/*   return original_sleep(seconds); */
/* } */

static void init() __attribute__((constructor));

void init()
{
  void *handle;
  handle = dlopen("libc.so.6", RTLD_LAZY);
  libc_sleep = dlsym(handle, "sleep");
  printf("in init function of %s\n", __FILE__);
  libc_sleep(3);
  srand(time(NULL));
}
