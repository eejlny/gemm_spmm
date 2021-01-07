#ifndef KERNELMATRIXMULT_H_
#define KERNELMATRIXMULT_H_

int kernelMatrixmult1(
ap_uint<2> ternary,
DTYPE *array_a,
DTYPE *array_b,
DTYPE_OUT *array_c,
int M,
int P,
int begin,
int end);


#endif 
