/* File: matrix_mult_accel.cpp
 *
 Copyright (c) [2016] [Mohammad Hosseinabady (mohammad@hosseinabady.com)]

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
* for the ENPOWER project funded by EPSRC
*
* File name : matrix_mult_accel.cpp
* author    : Mohammad hosseinabady mohammad@hosseinabady.com
* date      : 1 October 2016
* blog: https://highlevel-synthesis.com/
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix_mult.h"

inline void mxv(ap_uint<2> ternary,int M, DTYPE* a, DTYPE_OUT* c, DTYPE [B_HEIGHT][B_WIDTH_BLOCK]);
typedef unsigned long u32;

// note that  BLOCK shoudl be less than B_WIDTH_BLOCK
const int BLOCK=B_WIDTH_BLOCK;   //BLOCK should be less than B_WIDTH_BLOCK
const int STEP=1;

/*
 The amount of data saved in the FPGA is B_HEIGHT*B_WIDTH_BLOCK+A_WIDTH+B_WIDTH_BLOCK which should be less than FPGA BRAM size
*/
//#pragma SDS data sys_port(A:ACP)
#pragma SDS data zero_copy(A[0:N*M],B[0:M*P], C[0:N*P])
int mmult_top(ap_uint<2> ternary, int N, int M, int P, DTYPE* A, DTYPE* B, DTYPE_OUT* C)
{
	 DTYPE A_accel[A_WIDTH], B_accel[B_HEIGHT][B_WIDTH_BLOCK];
	 DTYPE_OUT C_accel[B_WIDTH_BLOCK];
	 #pragma HLS array_partition variable=A_accel block factor= 16 dim=1
	 #pragma HLS array_partition variable=B_accel block factor= 16 dim=2
	 //#pragma HLS array_partition variable=B_accel block factor= 16 dim=2
	 //#pragma HLS array_partition variable=A_accel block factor= 4 dim=1
     //#pragma HLS array_partition variable=B_accel block factor= 4 dim=2
     //#pragma HLS array_partition variable=A_accel complete
     //#pragma HLS array_partition variable=B_accel complete
	 //notice that array a_accel has only 1 dim and B has two dim dim 1 is the rows and dim 2 is columns.
	 #pragma HLS array_partition variable=C_accel complete



	 for (int B_index = 0; B_index < P/B_WIDTH_BLOCK; B_index++) {

		 for (int i = 0; i < M; i++) {
					for (int j = 0; j < B_WIDTH_BLOCK; j++) {
						B_accel[i][j] = B[i*P+j+B_index*B_WIDTH_BLOCK];
						//std::cout << i << " " << j << std::endl;
					}
		 }


		 for (int A_index = 0; A_index < N; A_index++) {
               #pragma HLS loop_tripcount min=64 max=64 avg=64

				for (int j = 0; j < M; j++) {
					A_accel[j] = A[A_index*M+j];
				}

				 mmult_accel(ternary,M, A_accel, B_accel, C_accel);

				 for (int i = 0; i < C_HEIGHT_BLOCK; i++) {
					for (int j = 0; j < C_WIDTH_BLOCK; j++) {
						C[(i+A_index*A_HEIGHT_BLOCK)*P+j+B_index*B_WIDTH_BLOCK] = C_accel[i*C_WIDTH_BLOCK+j];
						 //std::cout << "C is " << C[(i+A_index*A_HEIGHT_BLOCK)*C_WIDTH+j+B_index*B_WIDTH_BLOCK] << std::endl;
					}
				 }

	    	 }
	     }
}

int mmult_accel(ap_uint<2> ternary, int M, DTYPE* A, DTYPE B[B_HEIGHT][B_WIDTH_BLOCK], DTYPE_OUT* C)
{

	//int b[B_HEIGHT][B_WIDTH_BLOCK];
//#pragma HLS ARRAY_PARTITION variable=b complete dim=2

/*
	for (int i = 0; i < B_HEIGHT; i++) {
		for (int j = 0; j < B_WIDTH_BLOCK; j++) {
	#pragma HLS PIPELINE
			b[i][j] = *(B+i*B_WIDTH_BLOCK+j);
		}
	}*/


	for (int p = 0; p < A_HEIGHT_BLOCK; p+=STEP) {
		mxv(ternary,M,A+p*M, C+p*C_WIDTH_BLOCK, B);
	}


	return 0;
}



inline void mxv(ap_uint<2> ternary,int M, DTYPE *A, DTYPE_OUT* C, DTYPE B[B_HEIGHT][B_WIDTH_BLOCK])  {


	//int a[A_WIDTH];
	//int c[B_WIDTH_BLOCK];
	//DTYPE acc[C_WIDTH_BLOCK];
	//const int width = C_WIDTH_BLOCK;
	//#pragma HLS resource variable=acc[width] core=AddSub_DSP
    //#pragma HLS ARRAY_PARTITION variable=c complete dim=1
    //#pragma HLS array_partition variable=a block factor=16

	//memcpy(a,(int *)(A),A_WIDTH*sizeof(int));


	//for (int i = 0; i < A_WIDTH; i++) {
	//	a[i] = A[i];
	//}

	for (int j = 0; j < C_WIDTH_BLOCK; j++) {
	#pragma HLS UNROLL
		C[j] = 0;
		//acc[j] = 0;
	}



     //#pragma HLS PIPELINE

	 if(ternary==0)
	 {

	   for(int k = 0; k < M; k+=1) {
	        #pragma HLS PIPELINE
	        //#pragma HLS UNROLL factor=BLOCK


			 for (int j = 0; j < B_WIDTH_BLOCK; j++) {
	         //#pragma HLS PIPELINE
	         //#pragma HLS UNROLL factor=BLOCK
				//C[j] += A[k]*b[k][j];


				for(int z = 0; z < DTYPE_LENGTH; z+=8) {
	  				ap_int<8> A_val = A[k].range(z+7,z);
	  				C[j] += A_val*B[k][j];
				}

				//std::cout << "C is" << C[j] << std::endl;
				//C[j] = acc[j];
			}

		}
	 }
	 else if (ternary==1)
	 {

 	   for(int k = 0; k < M; k+=1) {
        #pragma HLS PIPELINE
        //#pragma HLS UNROLL factor=BLOCK


		 for (int j = 0; j < B_WIDTH_BLOCK; j++) {



 			for(int z = 0; z < DTYPE_LENGTH; z+=2) {
  			  				ap_int<2> A_val = A[k].range(z+1,z);
							//DTYPE A_val = A[k].range(z+1,z);
 			       			//DTYPE A_temp = A_val;

  			       			ap_int<2> B_temp,C_val;
  			       		    B_temp = B[k][j].range(z+1,z);
							C_val = A_val*B_temp;

							C[j] += C_val;


		        }
			}
			//std::cout << "C is" << C[j] << std::endl;
			//C[j] = acc[j
 	   }
	}
	else
	{

 	   for(int k = 0; k < M; k+=1) {
        #pragma HLS PIPELINE
        //#pragma HLS UNROLL factor=BLOCK


		 for (int j = 0; j < B_WIDTH_BLOCK; j++) {



  			for(int z = 0; z < DTYPE_LENGTH; z+=4) {
  			  				ap_int<4> A_val = A[k].range(z+3,z);
							//DTYPE A_val = A[k].range(z+1,z);
 			       			//DTYPE A_temp = A_val;

  			       			ap_int<4> B_temp,C_val;
  			       		    B_temp = B[k][j].range(z+3,z);
							C_val = A_val*B_temp;

							C[j] += C_val;

		        }
			}
			//std::cout << "C is" << C[j] << std::endl;
			//C[j] = acc[j
 	   }
	}
	//std::cout << "C is" << *C << std::endl;
}

//
//	for (int i = 0; i < B_WIDTH_BLOCK; i++) {
//		C[i] = c[i];
//	}

	//memcpy((int *)(C), c, C_WIDTH_BLOCK*sizeof(int));

	//return;
//}
