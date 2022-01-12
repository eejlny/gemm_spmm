/* File: spmv_mohammad.h
 *
 Copyright (c) [2019] [Mohammad Hosseinabady (mohammad@hosseinabady.com)]

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
* for the ENEAC project (Jose Nunez-Yanez) funded by EPSRC
*
* File name : spmv_mohammad.h
* author    : Mohammad hosseinabady mohammad@hosseinabady.com
* date      : 18 February 2019
* blog: https://highlevel-synthesis.com/
*/

#ifndef __SPMV_MOHAMMAD__
#define __SPMV_MOHAMMAD__

/*
 * for clock frequency 100 MHZ   II=4
 * for clock frequency 200 MHZ   II=8
 * for clock frequency 300 MHZ   II=10
 */
//const static int II = 4;

const static int II = 1;

typedef ap_uint<32> DTYPE;
typedef ap_int<32> DTYPE_OUT;


const static int ROW_SIZE_MAX        = (1024);
const static int ROW_SIZE_THREAD_MAX = (1024);
const static int COL_SIZE_MAX        = (1024);


void spmm_wrapper(
		ap_uint<2> ternary,
		int *rowPtr,

		int *columnIndex,

		DTYPE *values,

		DTYPE *y,

		DTYPE *x,
		int        no_vectors,

		int col_size,
		int row_size,
		int nnz,

		int begin,
		int end,


		int first_rowPrt_value,

                ap_int<32> *quantized_multiplier,
                ap_int<32> *shift,
                ap_int<32> *bias,
                ap_int<32> bias_count,
                ap_int<8> zero_point_lhs,
                ap_int<8> zero_point_rhs,
                ap_int<8> zero_point_dst,
		ap_int<8> clamp_max,
		ap_int<8> clamp_min
 

		);



void spmm(
		ap_uint<2> ternary,
		int *rowPtr,

		int *columnIndex,

		DTYPE *values,
		DTYPE *y,
		DTYPE *x,
		int        nv,

		int col_size,
		int row_size,
		int nnz,

		int begin,
		int end,

		int first_rowPrt_value,
		ap_int<32> quantized_multiplier[1024],
		ap_int<32> shift[1024],
		ap_int<32> bias_data[1024],

		ap_uint<8> zero_point_lhs,  
		ap_uint<8> zero_point_rhs, 
		ap_uint<8> zero_point_dst,
		ap_uint<8> clamp_max,
		ap_uint<8> clamp_min


		);



#endif
