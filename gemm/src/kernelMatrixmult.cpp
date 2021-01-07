#include <stdio.h>
#include <stdlib.h>
//#include <sds_lib.h>
#include "matrix_mult.h"

int kernelMatrixmult1(
ap_uint<2> ternary,
DTYPE *array_a,
DTYPE *array_b,
DTYPE_OUT *array_c,
int M,
int P,
int begin,
int end)
{

  int k;
  int line_count = end-begin;
  DTYPE *array_temp_a = array_a + begin*M;
  DTYPE *array_temp_b = array_b;
  DTYPE_OUT *array_temp_c = array_c + begin*M;
  //mmult_top1(array_temp_a, array_temp_b, array_temp_c, line_count);
  // #pragma SDS resource(1)
   mmult_top(ternary,line_count, M, P, array_temp_a, array_temp_b, array_temp_c);
}
