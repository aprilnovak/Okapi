#include "ExtraFunctions.h"

/* extra function for computing factorials for Zernike polynomials,
   courtesy of cplusplus.com. */
int factorial(int n)
{
  return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}

/* compute the number of Zernike polynomials given the expansion order
   N (not so simple as multiplying n * m because m can only have values
   m = -n, -n + 2, -n + 4, ..., n. */
int num_zernike(int N)
{
  int number = 0;

  for (int i = 1; i <= N + 1; ++i)
    number += i;

  return number;
}
