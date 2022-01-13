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
* File name : matrix_mult.h
* author    : Jose Nunez-Yanez eejlny@bristol.ac.uk
* date      : 1 October 2021
*/


#ifndef KERNELMATRIXMULT_H_
#define KERNELMATRIXMULT_H_

int kernelgemm1(
ap_uint<2> ternary,
ap_int<32> *quantized_multiplier,
ap_int<32> *shift,
ap_int<32> *bias,
ap_int<32> bias_count,
ap_int<8> zero_point_lhs,
ap_int<8> zero_point_rhs,
ap_int<8> zero_point_dst,
ap_int<8> clamp_max,
ap_int<8> clamp_min,
DTYPE *array_a,
DTYPE *array_b,
DTYPE *array_c,
int M,
int P,
int begin,
int end);

int kernelspmm1(
		int ternary,
		DATA_TYPE *values,
		int       *colIndices,
		int       *rowPtr,
		DATA_TYPE_X *x,
		int        no_vectors,
		DATA_TYPE *y,
		int        rows,
		int        cols,
		int        nnz,
    		ap_int<32> *quantized_multiplier, 
                ap_int<32> *shift, 
                ap_int<32> *bias,  
                ap_int<32> bias_count, 
                ap_int<8> zero_point_lhs,  
                ap_int<8> zero_point_rhs, 
                ap_int<8> zero_point_dst,
		ap_int<8> clamp_max,
		ap_int<8> clamp_min,
		int        begin,
		int        end
);



//int kernelMatrixmult2(
//ap_uint<2> ternary,
//ap_int<32> quantized_multiplier,
//ap_int<32> shift,
//ap_int<32> *bias,
//ap_int<32> bias_count,
//ap_uint<8> zero_point_lhs,
//ap_uint<8> zero_point_rhs,
//ap_uint<8> zero_point_dst,
//ap_uint<8> clamp_max,
//ap_uint<8> clamp_min,
//DTYPE *array_a,
//DTYPE *array_b,
//DTYPE *array_c,
//int M,
//int P,
//int begin,
//int end);


#endif 
