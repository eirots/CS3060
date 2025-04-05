#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Missing hex address, input was: %s\t", argv[0]);
    return 1;
  }

  char *endptr;
  unsigned long address = strtoul(argv[1], &endptr, 16);

  unsigned long index = address >> 12;
  unsigned long offset = address & 0xFFF;

  printf("Logical Addr: 0x%08lX - Page Index: 0x%08lX - Offset: 0x%08lX \n",
         address, index, offset);

  return 0;
}