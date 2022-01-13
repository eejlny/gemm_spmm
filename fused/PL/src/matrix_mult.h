/* File: matrix_mult.h
 *
 Copyright (c) [2021] [Jose Nunez-Yanez (eejlny@bristol.ac.uk)]

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

#ifndef __MATRIX_MULT_H__
#define __MATRIX_MULT_H__

#include <vector>

// 64 * 16(values in 32-bit word (ternary)) = 1024
//1536 and 16 in a word so 96 words of 32-bit
//1536 and 4 in a word so 384 words of 8-bit

//if you want to generate CSR then set this DTYPE_LENGTH to 8 since spmm uses packing to 8-bit for A matrix
// if you set it to 32 then A matrix will be formed with 32-bits words instead of 8-bit
//8-bit packs
// ./generate_csr_8.elf /mnt/weights_tern_090_block_32_1.csv 1 6 336  512
//32-bit packs
// ./generate_csr_32.elf /mnt/weights_tern_090_block_32_1.csv 1 6 84  512
//quad
//8-bit packs
// ./generate_csr_8.elf /mnt/weights_quad_block_095_32_1.csv 2 6 672  512
//32-bit packs
// ./generate_csr_32.elf /mnt/weights_quad_block_095_32_1.csv 2 6 168  512

//#define GENERATE_CSR 0
#define DTYPE_LENGTH 32 //8//32 //8 //32//32 //8//32//8
#define MAX_N    2048 //2048 //64
//#define MAX_M    1536 //2048 //384 //1536 //384 //1536 // 384 //48 //768 //96 //384 //96 //384 //96 //384// 96
#define MAX_M    512 //2048 //384 //1536 //384 //1536 // 384 //48 //768 //96 //384 //96 //384 //96 //384// 96
#define MAX_P    32768 //2048 //512 //64//1//64//1

//#define SN    64 //2048 //64
//#define SM    384 //1536 //384 //1536 //384 //1536 // 384 //48 //768 //96 //384 //96 //384 //96 //384// 96
//#define SP    512//64//1//64//1


//48 for WL 64
//96 for WL 32
// 384 for WL 8
// 768 for WL 4

#define A_HEIGHT   MAX_N
#define A_WIDTH    MAX_M

#define B_HEIGHT   MAX_M
#define B_WIDTH    MAX_P

#define C_HEIGHT   MAX_N
#define C_WIDTH    MAX_P


typedef ap_uint<DTYPE_LENGTH> DTYPE;
typedef ap_int<32> DTYPE_OUT;


#define A_HEIGHT_BLOCK  1// 4096 //(512/4)
#define B_WIDTH_BLOCK 128 //16 //32 //64 //64 //128 // 64 //64 //64 //8//8// //16//32//1//32//1//32//1// 1//32//(128/4)
#define B_BLOCK_PARALLEL 1

#define ENABLE_GEMM
#define ENABLE_SPMM
#define ENABLE_SCALING
#define ENABLE_TRANSPOSE


//typedef ap_int<32> c_fifo_t;
//typedef hls::stream<c_fifo_t> c_fifo_stream_t;

//how many rows of B are computed in parallel in multiplication loop
//for example a couple of B rows are multiplied for A 1 row in each loop iteration
//it basically reduces how the loop iterations by 2 if it is 2.
/*
 A_HEIGHT_BLOCK  is for software part data partitioning due to the limitation in 
 the Xilinx kernel sds_alloc so A_HEIGHT_BLOCK should be A_HEIGHT divided by 
 the number of considered blocks
*/
#define C_HEIGHT_BLOCK  A_HEIGHT_BLOCK 


#define C_WIDTH_BLOCK   B_WIDTH_BLOCK //B_WIDTH_BLOCK shoudl be less than P

//typedef unsigned long u32;

typedef std::vector<int> vi;



int mmult_accel(ap_uint<2> ternary, ap_int<8> zero_point_lhs,  ap_int<8> zero_point_rhs, int M, DTYPE A[A_HEIGHT_BLOCK][A_WIDTH], DTYPE B[B_HEIGHT][B_WIDTH_BLOCK], DTYPE_OUT C[C_HEIGHT_BLOCK][B_WIDTH_BLOCK]);
//#pragma SDS data access_pattern(A:SEQUENTIAL)


//#pragma SDS data zero_copy(B[0:B_HEIGHT*B_WIDTH], C[0:A_HEIGHT*B_WIDTH])
//#pragma SDS data copy(A[0:A_HEIGHT*A_WIDTH])
//#pragma SDS data access_pattern(A:RANDOM)
//#pragma SDS data zero_copy(A[0:A_HEIGHT*A_WIDTH],B[0:B_HEIGHT*B_WIDTH], C[0:A_HEIGHT*B_WIDTH])
//#pragma SDS data sys_port(A:AFI,B:AFI,C:AFI)
//#pragma SDS data access_pattern(A:ACP;B:ACP;C:ACP)
//#pragma SDS data zero_copy(A[0:line_count*A_WIDTH],B[0:B_HEIGHT*B_WIDTH], C[0:A_HEIGHT*B_WIDTH])
int mmult_top(ap_uint<2> ternary,ap_int<32> *quantized_multiplier, ap_int<32> *shift, ap_int<32> *bias, ap_int<32> bias_count,ap_int<8> zero_point_lhs,  ap_int<8> zero_point_rhs, ap_int<8> zero_point_dst,ap_int<8> clamp_max,ap_int<8> clamp_min,int N, int M, int P, DTYPE* A, DTYPE* B, DTYPE* C);
//int mmult_top2(int A[A_WIDTH*A_HEIGHT], int B[B_HEIGHT*B_WIDTH], int C[C_HEIGHT*C_WIDTH],int line_count);
//int mmult_top3(int A[A_WIDTH*A_HEIGHT], int B[B_HEIGHT*B_WIDTH], int C[C_HEIGHT*C_WIDTH],int line_count);
//int mmult_top4(int A[A_WIDTH*A_HEIGHT], int B[B_HEIGHT*B_WIDTH], int C[C_HEIGHT*C_WIDTH],int line_count);

#endif //__MATRIX_MULT_H__



