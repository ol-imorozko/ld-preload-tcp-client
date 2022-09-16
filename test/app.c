#include <stdio.h>
#include <unistd.h>

int main()
{
  int i;
  printf("in main()\n");

  for (int i = 0; i < 100; i++)
  {
    sleep(1);
    printf("zzz...\n\n");
  }
  return 0;
}
