#include "ExtraFunctions.h"

/* extra function for computing factorials for Zernike polynomials,
   courtesy of cplusplus.com. */
int factorial(int n)
{
  return (n == 1 || n == 0) ? 1 : factorial(n - 1) * n;
}
