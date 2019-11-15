#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main() {
  srand(getpid());
  float b = cosf((float)rand()/(float)RAND_MAX);
  printf("resultat = %g\n", b);
}
