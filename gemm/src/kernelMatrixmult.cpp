/*Copyright (c) [2021] [Jose Nunez-Yanez (eejlny@bristol.ac.uk)]

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
===============================================================================
* This file has been written at University of Bristol
* for the HOPWARE/MINET project
*
* 
* author    : Jose Nunez-Yanez eejlny@bristol.ac.uk
* date      : 1 October 2021
*/

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
