/*
   32-bits Random number generator U(0,1): MRG32k3a
   Author: Pierre L'Ecuyer,
   Source: Good Parameter Sets for Combined Multiple Recursive Random
           Number Generators,
           Shorter version in Operations Research,
           47, 1 (1999), 159--164.
   ---------------------------------------------------------
*/
#include "MRG32k3a.h"
#include <stdio.h>

#define norm 2.328306549295728e-10
#define m1   4294967087.0
#define m2   4294944443.0
#define a12     1403580.0
#define a13n     810728.0
#define a21      527612.0
#define a23n    1370589.0

/***
The seeds for s10, s11, s12 must be integers in [0, m1 - 1] and not all 0. 
The seeds for s20, s21, s22 must be integers in [0, m2 - 1] and not all 0. 
***/

static double s10, s11, s12,
              s20, s21, s22;

void mrg_seed(unsigned int s10p, unsigned int s11p, unsigned int s12p,
              unsigned int s20p, unsigned int s21p, unsigned int s22p) {

    unsigned int lm1 = m1, lm2 = m2;

    // adding 1 to each seed to guarantee all seeds will never be 0
    s10 = (s10p % lm1)+1; s11 = (s11p % lm1)+1; s12 = (s12p % lm1)+1;
    s20 = (s20p % lm2)+1; s21 = (s21p % lm2)+1; s22 = (s22p % lm2)+1;
}

void mrg_seed_array(unsigned int* a) {
    mrg_seed(a[0], a[1], a[2],
             a[3], a[4], a[5]);
}

double MRG32k3a (void)
{
   long k;
   double p1, p2;
   /* Component 1 */
   p1 = a12 * s11 - a13n * s10;
   k = p1 / m1;
   p1 -= k * m1;
   if (p1 < 0.0)
      p1 += m1;
   s10 = s11;
   s11 = s12;
   s12 = p1;

   /* Component 2 */
   p2 = a21 * s22 - a23n * s20;
   k = p2 / m2;
   p2 -= k * m2;
   if (p2 < 0.0)
      p2 += m2;
   s20 = s21;
   s21 = s22;
   s22 = p2;

   /* Combination */
   if (p1 <= p2)
      return ((p1 - p2 + m1) * norm);
   else
      return ((p1 - p2) * norm);
}
